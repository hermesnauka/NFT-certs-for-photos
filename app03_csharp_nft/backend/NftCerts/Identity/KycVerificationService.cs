using NftCerts.Db;

namespace NftCerts.Identity;

// Swap-in point for a real DID/KYC provider (Persona, Civic, ...). Mirrors app01's interface.
public interface IKycVerificationService
{
    ArtistIdentity Verify(string walletAddress, string did, string email);
    bool IsVerified(string walletAddress);
}

// Mock KYC provider integration: always approves verification. Stands in for a real provider
// that would be wired in a production deployment.
public class MockKycVerificationService(NftCertsDbContext db) : IKycVerificationService
{
    public ArtistIdentity Verify(string walletAddress, string did, string email)
    {
        var identity = db.ArtistIdentities.Find(walletAddress) ?? new ArtistIdentity { WalletAddress = walletAddress };
        identity.Did = did;
        identity.Email = email;
        identity.Verified = true;
        identity.VerifiedAt = DateTime.UtcNow.ToString("yyyy-MM-dd'T'HH:mm:ss'Z'");
        db.ArtistIdentities.Update(identity);
        db.SaveChanges();
        return identity;
    }

    public bool IsVerified(string walletAddress) =>
        db.ArtistIdentities.Find(walletAddress)?.Verified ?? false;
}
