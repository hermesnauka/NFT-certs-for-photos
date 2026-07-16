using System.ComponentModel.DataAnnotations;
using Microsoft.EntityFrameworkCore;

namespace NftCerts.Certificates;

public enum ArtworkStatus
{
    UPLOADED,
    PINNED,
    MINTED,
}

// Mirrors app01's Artwork JPA entity: an uploaded photograph after hashing and watermarking,
// before (or after) it has been pinned to IPFS and minted.
[Index(nameof(Sha256Hash), IsUnique = true)]
public class Artwork
{
    [Key]
    public string Id { get; set; } = "";          // UUID
    public string FileId { get; set; } = "";       // UUID
    public string OriginalFilename { get; set; } = "";
    public string Sha256Hash { get; set; } = "";   // hex, 64 chars, unique
    public string Title { get; set; } = "";
    public string Description { get; set; } = "";
    public string Medium { get; set; } = "";
    public int? YearCreated { get; set; }
    public int RoyaltyPercentageBps { get; set; }
    public string ArtistWalletAddress { get; set; } = "";
    public string ArtistDid { get; set; } = "";
    public string ImageIpfsUri { get; set; } = "";
    public string MetadataIpfsUri { get; set; } = "";
    public ArtworkStatus Status { get; set; } = ArtworkStatus.UPLOADED;
    public string CreatedAt { get; set; } = "";    // ISO-8601 instant
}
