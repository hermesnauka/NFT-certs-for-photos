using Microsoft.EntityFrameworkCore;
using NftCerts.Certificates;
using NftCerts.Db;
using NftCerts.Errors;
using NftCerts.Identity;
using NftCerts.Storage;
using NftCerts.Tests.TestSupport;

namespace NftCerts.Tests.Services;

public class CertificateServiceTests : IDisposable
{
    private readonly TestDb _db = new();
    private readonly LocalStubIpfsStorageService _storage = new();
    private readonly MockKycVerificationService _kyc;
    private readonly FakeContractService _contractService = new();
    private readonly CertificateService _service;

    public CertificateServiceTests()
    {
        _kyc = new MockKycVerificationService(_db.Context);
        _service = new CertificateService(_db.Context, _storage, _contractService, _kyc);
    }

    public void Dispose() => _db.Dispose();

    private static StoredUpload SampleUpload(string sha256 = "aa11bb22cc33dd44ee55ff66aa11bb22cc33dd44ee55ff66aa11bb22cc33dd4") =>
        new("photo.jpg", "image/jpeg", sha256, [1, 2, 3, 4]);

    [Theory]
    [InlineData(-1)]
    [InlineData(10001)]
    public void CreateArtwork_RoyaltyOutOfRange_Throws(int royaltyBps)
    {
        Assert.Throws<RoyaltyOutOfRangeException>(() =>
            _service.CreateArtwork(SampleUpload(), "Title", "desc", "medium", 2024, royaltyBps, "0xWallet", "did:key:z"));
    }

    [Fact]
    public void CreateArtwork_ValidInput_PinsAndPersistsAsPinned()
    {
        Artwork artwork = _service.CreateArtwork(SampleUpload(), "Title", "desc", "medium", 2024, 500,
                                                  "0xWallet", "did:key:z");

        Assert.Equal(ArtworkStatus.PINNED, artwork.Status);
        Assert.StartsWith("ipfs://stub-", artwork.ImageIpfsUri);
        Assert.StartsWith("ipfs://stub-", artwork.MetadataIpfsUri);
        Assert.Single(_db.Context.Artworks);
    }

    [Fact]
    public void MintCertificate_UnknownArtworkId_ThrowsNotFound()
    {
        Assert.Throws<ArtworkNotFoundException>(() => _service.MintCertificate("missing-id", "0xRecipient"));
    }

    [Fact]
    public void MintCertificate_ArtworkNotYetPinned_ThrowsUnprocessable()
    {
        var artwork = new Artwork
        {
            Id = "artwork-1", Sha256Hash = "hash1", ArtistWalletAddress = "0xWallet",
            Status = ArtworkStatus.UPLOADED, CreatedAt = "2026-01-01T00:00:00Z",
        };
        _db.Context.Artworks.Add(artwork);
        _db.Context.SaveChanges();

        Assert.Throws<ArtworkNotPinnedException>(() => _service.MintCertificate("artwork-1", "0xRecipient"));
    }

    [Fact]
    public void MintCertificate_ArtistNotVerified_ThrowsForbidden()
    {
        Artwork artwork = _service.CreateArtwork(SampleUpload(), "Title", "desc", "medium", 2024, 500,
                                                  "0xUnverifiedWallet", "did:key:z");

        Assert.Throws<NotVerifiedException>(() => _service.MintCertificate(artwork.Id, "0xRecipient"));
    }

    [Fact]
    public void MintCertificate_DuplicateContentHash_ThrowsConflict()
    {
        // Artwork.Sha256Hash carries a DB-level unique index (SQLite), so two Artwork rows sharing
        // one hash can never coexist there — this state is unreachable via the real (SQLite-backed)
        // app both through the API and through direct persistence. The EF Core InMemory provider
        // doesn't enforce that relational index, so it's used here purely to isolate and assert
        // CertificateService.MintCertificate's own duplicate-hash guard against the Certificates
        // table, independent of that outer constraint.
        const string sharedHash = "aa11bb22cc33dd44ee55ff66aa11bb22cc33dd44ee55ff66aa11bb22cc33dd4";
        var options = new DbContextOptionsBuilder<NftCertsDbContext>()
            .UseInMemoryDatabase(Guid.NewGuid().ToString()).Options;
        using var db = new NftCertsDbContext(options);
        var kyc = new MockKycVerificationService(db);
        var service = new CertificateService(db, new LocalStubIpfsStorageService(), new FakeContractService(), kyc);

        kyc.Verify("0xWallet", "did:key:z", "artist@example.com");
        Artwork first = service.CreateArtwork(SampleUpload(sharedHash), "Title", "desc", "medium", 2024, 500,
                                               "0xWallet", "did:key:z");
        service.MintCertificate(first.Id, "0xRecipient");

        var second = new Artwork
        {
            Id = "artwork-2", Sha256Hash = sharedHash, ArtistWalletAddress = "0xWallet",
            Status = ArtworkStatus.PINNED, CreatedAt = "2026-01-01T00:00:00Z",
        };
        db.Artworks.Add(second);
        db.SaveChanges();

        Assert.Throws<DuplicateContentHashException>(() => service.MintCertificate(second.Id, "0xRecipient"));
    }

    [Fact]
    public void MintCertificate_HappyPath_PersistsCertificateAndMarksArtworkMinted()
    {
        _kyc.Verify("0xWallet", "did:key:z", "artist@example.com");
        Artwork artwork = _service.CreateArtwork(SampleUpload(), "Title", "desc", "medium", 2024, 500,
                                                  "0xWallet", "did:key:z");

        Certificate certificate = _service.MintCertificate(artwork.Id, "0xRecipient");

        Assert.Equal(1, certificate.TokenId);
        Assert.Equal("0xRecipient", certificate.OwnerAddress);
        Assert.Equal(artwork.Sha256Hash, certificate.ContentHashHex);
        Assert.Equal(ArtworkStatus.MINTED, _db.Context.Artworks.Find(artwork.Id)!.Status);
    }

    [Fact]
    public void GetCertificate_UnknownTokenId_ThrowsNotFound()
    {
        Assert.Throws<CertificateNotFoundException>(() => _service.GetCertificate(999));
    }

    [Fact]
    public void GetCertificatesForWallet_ReturnsOnlyThatWalletsCertificates()
    {
        _kyc.Verify("0xWallet", "did:key:z", "artist@example.com");
        Artwork artwork = _service.CreateArtwork(SampleUpload(), "Title", "desc", "medium", 2024, 500,
                                                  "0xWallet", "did:key:z");
        _service.MintCertificate(artwork.Id, "0xRecipient");

        List<Certificate> forRecipient = _service.GetCertificatesForWallet("0xRecipient");
        List<Certificate> forOther = _service.GetCertificatesForWallet("0xSomeoneElse");

        Assert.Single(forRecipient);
        Assert.Empty(forOther);
    }
}
