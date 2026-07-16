using NftCerts.Identity;
using NftCerts.Tests.TestSupport;

namespace NftCerts.Tests.Services;

public class MockKycVerificationServiceTests
{
    [Fact]
    public void IsVerified_UnknownWallet_ReturnsFalse()
    {
        using var db = new TestDb();
        var service = new MockKycVerificationService(db.Context);

        Assert.False(service.IsVerified("0xUnknown"));
    }

    [Fact]
    public void Verify_AlwaysApproves_AndPersistsIdentity()
    {
        using var db = new TestDb();
        var service = new MockKycVerificationService(db.Context);

        ArtistIdentity identity = service.Verify("0xWallet1", "did:key:zabc", "artist@example.com");

        Assert.True(identity.Verified);
        Assert.Equal("did:key:zabc", identity.Did);
        Assert.True(service.IsVerified("0xWallet1"));
    }

    [Fact]
    public void Verify_CalledTwiceForSameWallet_UpdatesExistingRecord()
    {
        using var db = new TestDb();
        var service = new MockKycVerificationService(db.Context);

        service.Verify("0xWallet1", "did:key:zabc", "old@example.com");
        ArtistIdentity updated = service.Verify("0xWallet1", "did:key:znew", "new@example.com");

        Assert.Equal("did:key:znew", updated.Did);
        Assert.Single(db.Context.ArtistIdentities);
    }
}
