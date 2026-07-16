using System.ComponentModel.DataAnnotations;

namespace NftCerts.Identity;

// Mirrors app01's ArtistIdentity JPA entity.
public class ArtistIdentity
{
    [Key]
    public string WalletAddress { get; set; } = "";
    public string Did { get; set; } = "";
    public string Email { get; set; } = "";
    public bool Verified { get; set; }
    public string VerifiedAt { get; set; } = "";  // ISO-8601 instant
}
