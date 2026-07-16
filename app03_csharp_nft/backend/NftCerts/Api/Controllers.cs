using System.Text.Json;
using System.Text.RegularExpressions;
using Microsoft.AspNetCore.Mvc;
using NftCerts.Certificates;
using NftCerts.Errors;
using NftCerts.Hashing;
using NftCerts.I18n;
using NftCerts.Identity;
using NftCerts.Pdf;
using NftCerts.Watermark;

namespace NftCerts.Api;

// POST /api/uploads — multipart file upload: hash, watermark, stage in the UploadStore.
[ApiController]
[Route("api/uploads")]
public class UploadController(
    Sha256HashingService hashingService,
    MetadataWatermarkService watermarkService,
    UploadStore uploadStore) : ControllerBase
{
    private static readonly Dictionary<string, string> SupportedExtensions = new(StringComparer.OrdinalIgnoreCase)
    {
        [".jpg"] = "image/jpeg",
        [".jpeg"] = "image/jpeg",
        [".png"] = "image/png",
        [".webp"] = "image/webp",
    };

    [HttpPost]
    public IActionResult Upload(IFormFile? file)
    {
        if (file is null || file.Length == 0)
        {
            throw new EmptyFileException();
        }

        string extension = Path.GetExtension(file.FileName);
        if (!SupportedExtensions.TryGetValue(extension, out var contentType))
        {
            throw new UnsupportedFileTypeException(extension.TrimStart('.'));
        }

        using var buffer = new MemoryStream();
        file.CopyTo(buffer);
        byte[] originalContent = buffer.ToArray();

        string sha256Hash = hashingService.Sha256Hex(originalContent);
        // The artist DID is not yet known at upload time (supplied later on /api/artworks), so an
        // empty marker is used here — mirrors app01's UploadController.
        byte[] watermarkedContent = watermarkService.Watermark(originalContent, contentType, sha256Hash, "");

        string fileId = uploadStore.Store(new StoredUpload(file.FileName, contentType, sha256Hash, watermarkedContent));

        return Ok(new
        {
            fileId,
            originalFilename = file.FileName,
            sha256Hash,
            sizeBytes = watermarkedContent.Length,
        });
    }
}

public record CreateArtworkRequest(string? FileId, string? Title, string? Description, string? Medium,
                                    int? YearCreated, int? RoyaltyPercentageBps, string? ArtistWalletAddress,
                                    string? ArtistDid);

public record MintRequest(string? RecipientAddress);

// POST /api/artworks and POST /api/artworks/{artworkId}/mint.
[ApiController]
[Route("api/artworks")]
public class ArtworkController(
    UploadStore uploadStore,
    CertificateService certificateService,
    CertificateDtoMapper dtoMapper) : ControllerBase
{
    [HttpPost]
    public IActionResult Create([FromBody] CreateArtworkRequest? request)
    {
        if (request is null) throw new ValidationException("Request body must be valid JSON");
        string fileId = RequireString(request.FileId, "fileId");
        string title = RequireString(request.Title, "title");
        if (request.RoyaltyPercentageBps is null)
        {
            throw new ValidationException("royaltyPercentageBps: must not be null");
        }
        string artistWalletAddress = RequireString(request.ArtistWalletAddress, "artistWalletAddress");

        StoredUpload upload = uploadStore.Get(fileId);
        Artwork artwork = certificateService.CreateArtwork(
            upload, title, request.Description ?? "", request.Medium ?? "", request.YearCreated,
            request.RoyaltyPercentageBps, artistWalletAddress, request.ArtistDid ?? "");

        return StatusCode(StatusCodes.Status201Created, new
        {
            artworkId = artwork.Id,
            sha256Hash = artwork.Sha256Hash,
            imageIpfsUri = artwork.ImageIpfsUri,
            metadataIpfsUri = artwork.MetadataIpfsUri,
            status = artwork.Status.ToString(),
        });
    }

    [HttpPost("{artworkId}/mint")]
    public IActionResult Mint(string artworkId, [FromBody] MintRequest? request)
    {
        if (request is null) throw new ValidationException("Request body must be valid JSON");
        string recipientAddress = RequireString(request.RecipientAddress, "recipientAddress");

        Certificate certificate = certificateService.MintCertificate(artworkId, recipientAddress);
        return Ok(dtoMapper.ToDto(certificate));
    }

    private static string RequireString(string? value, string field) =>
        string.IsNullOrEmpty(value) ? throw new ValidationException($"{field}: must not be blank") : value;
}

// GET /api/certificates/{tokenId} and GET /api/certificates/{tokenId}/pdf.
[ApiController]
[Route("api/certificates")]
public class CertificateController(
    CertificateService certificateService,
    CertificateDtoMapper dtoMapper,
    CertificatePdfService pdfService) : ControllerBase
{
    [HttpGet("{tokenIdRaw}")]
    public IActionResult Get(string tokenIdRaw) =>
        Ok(dtoMapper.ToDto(certificateService.GetCertificate(ParseTokenId(tokenIdRaw))));

    [HttpGet("{tokenIdRaw}/pdf")]
    public IActionResult GetPdf(string tokenIdRaw)
    {
        long tokenId = ParseTokenId(tokenIdRaw);
        Certificate certificate = certificateService.GetCertificate(tokenId);
        byte[] pdf = pdfService.Generate(certificate, dtoMapper.BuildEtherscanUrl(certificate),
                                          dtoMapper.BuildOpenSeaUrl(certificate),
                                          dtoMapper.BuildRaribleUrl(certificate));
        Response.Headers.ContentDisposition = $"attachment; filename=\"certificate-{tokenId}.pdf\"";
        return File(pdf, "application/pdf");
    }

    // Non-numeric ids map to 404, not a framework 400 — same contract as app01/app02.
    private static long ParseTokenId(string raw) =>
        long.TryParse(raw, out long tokenId) ? tokenId : throw new CertificateNotFoundException(-1);
}

// GET /api/artists/{walletAddress}/dashboard.
[ApiController]
[Route("api/artists")]
public class ArtistDashboardController(
    CertificateService certificateService,
    CertificateDtoMapper dtoMapper) : ControllerBase
{
    [HttpGet("{walletAddress}/dashboard")]
    public IActionResult Dashboard(string walletAddress)
    {
        List<Certificate> certificates = certificateService.GetCertificatesForWallet(walletAddress);
        return Ok(new
        {
            walletAddress,
            certificates = certificates.Select(dtoMapper.ToDto),
            totalCertificates = certificates.Count,
        });
    }
}

// GET /api/i18n/{lang}.
[ApiController]
[Route("api/i18n")]
public class I18nController : ControllerBase
{
    [HttpGet("{lang}")]
    public IActionResult Get(string lang) =>
        Ok(Messages.ForLanguage(lang) ?? throw new UnsupportedLanguageException(lang));
}

public record VerifyIdentityRequest(string? WalletAddress, string? Did, string? Email);

// POST /api/identity/verify.
[ApiController]
[Route("api/identity")]
public partial class IdentityController(IKycVerificationService kycService) : ControllerBase
{
    [GeneratedRegex("^0x[a-fA-F0-9]{40}$")]
    private static partial Regex WalletAddressPattern();

    [GeneratedRegex("^did:[a-zA-Z0-9]+:.+$")]
    private static partial Regex DidPattern();

    [HttpPost("verify")]
    public IActionResult Verify([FromBody] VerifyIdentityRequest? request)
    {
        if (request?.WalletAddress is null || request.Did is null)
        {
            throw new MalformedIdentityException("walletAddress and did are required");
        }
        if (!WalletAddressPattern().IsMatch(request.WalletAddress))
        {
            throw new MalformedIdentityException($"Malformed wallet address: {request.WalletAddress}");
        }
        if (!DidPattern().IsMatch(request.Did))
        {
            throw new MalformedIdentityException($"Malformed DID: {request.Did}");
        }

        ArtistIdentity identity = kycService.Verify(request.WalletAddress, request.Did, request.Email ?? "");
        return Ok(new
        {
            verified = identity.Verified,
            did = identity.Did,
            walletAddress = identity.WalletAddress,
        });
    }
}
