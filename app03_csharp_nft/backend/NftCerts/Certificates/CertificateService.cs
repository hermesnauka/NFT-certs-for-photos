using System.Text.Json;
using NftCerts.Blockchain;
using NftCerts.Db;
using NftCerts.Errors;
using NftCerts.Identity;
using NftCerts.Storage;
using Microsoft.EntityFrameworkCore;

namespace NftCerts.Certificates;

// The hashed/watermarked bytes of an upload, retrievable by fileId when creating an artwork.
public record StoredUpload(string OriginalFilename, string ContentType, string Sha256Hash, byte[] Content);

// Orchestrates the artwork -> IPFS pinning -> on-chain minting -> Certificate persistence
// pipeline. Mirrors app01/app02's CertificateService.
public class CertificateService(
    NftCertsDbContext db,
    IIpfsStorageService ipfsStorageService,
    ContractService contractService,
    IKycVerificationService kycVerificationService)
{
    public Artwork CreateArtwork(StoredUpload upload, string title, string description, string medium,
                                  int? yearCreated, int? royaltyPercentageBps, string artistWalletAddress,
                                  string artistDid)
    {
        if (royaltyPercentageBps is null or < 0 or > 10000)
        {
            throw new RoyaltyOutOfRangeException(royaltyPercentageBps ?? -1);
        }

        var artwork = new Artwork
        {
            Id = Guid.NewGuid().ToString(),
            FileId = Guid.NewGuid().ToString(),
            OriginalFilename = upload.OriginalFilename,
            Sha256Hash = upload.Sha256Hash,
            Title = title,
            Description = description,
            Medium = medium,
            YearCreated = yearCreated,
            RoyaltyPercentageBps = royaltyPercentageBps.Value,
            ArtistWalletAddress = artistWalletAddress,
            ArtistDid = artistDid,
            Status = ArtworkStatus.UPLOADED,
            CreatedAt = NowIso8601(),
        };

        PinResult imagePin = ipfsStorageService.PinFile(upload.Content, upload.OriginalFilename, upload.ContentType);
        artwork.ImageIpfsUri = imagePin.Uri;

        var metadata = JsonSerializer.SerializeToElement(new
        {
            title = artwork.Title,
            description = artwork.Description,
            medium = artwork.Medium,
            yearCreated = artwork.YearCreated,
            artistDid = artwork.ArtistDid,
            sha256Hash = artwork.Sha256Hash,
            image = imagePin.Uri,
        });
        PinResult metadataPin = ipfsStorageService.PinJson(metadata, artwork.Id + "-metadata");
        artwork.MetadataIpfsUri = metadataPin.Uri;

        artwork.Status = ArtworkStatus.PINNED;

        db.Artworks.Add(artwork);
        db.SaveChanges();
        return artwork;
    }

    public Certificate MintCertificate(string artworkId, string recipientAddress)
    {
        Artwork artwork = db.Artworks.Find(artworkId) ?? throw new ArtworkNotFoundException(artworkId);

        if (artwork.Status != ArtworkStatus.PINNED)
        {
            throw new ArtworkNotPinnedException(artworkId);
        }
        if (!kycVerificationService.IsVerified(artwork.ArtistWalletAddress))
        {
            throw new NotVerifiedException(artwork.ArtistWalletAddress);
        }
        if (db.Certificates.Any(c => c.ContentHashHex == artwork.Sha256Hash))
        {
            throw new DuplicateContentHashException(artwork.Sha256Hash);
        }

        MintResult mintResult = contractService.MintCertificate(
            recipientAddress, artwork.MetadataIpfsUri, artwork.Sha256Hash, artwork.ArtistWalletAddress,
            artwork.RoyaltyPercentageBps);

        var certificate = new Certificate
        {
            TokenId = mintResult.TokenId,
            ArtworkId = artwork.Id,
            Artwork = artwork,
            ContentHashHex = artwork.Sha256Hash,
            ContractAddress = mintResult.ContractAddress,
            TxHash = mintResult.TxHash,
            OwnerAddress = recipientAddress,
            RoyaltyPercentageBps = artwork.RoyaltyPercentageBps,
            RoyaltyRecipient = artwork.ArtistWalletAddress,
            MintedAt = NowIso8601(),
        };

        artwork.Status = ArtworkStatus.MINTED;
        db.Certificates.Add(certificate);
        db.SaveChanges();
        return certificate;
    }

    public Certificate GetCertificate(long tokenId) =>
        db.Certificates.FirstOrDefault(c => c.TokenId == tokenId)
        ?? throw new CertificateNotFoundException(tokenId);

    public List<Certificate> GetCertificatesForWallet(string walletAddress) =>
        db.Certificates.Where(c => c.OwnerAddress == walletAddress).ToList();

    internal static string NowIso8601() => DateTime.UtcNow.ToString("yyyy-MM-dd'T'HH:mm:ss'Z'");
}
