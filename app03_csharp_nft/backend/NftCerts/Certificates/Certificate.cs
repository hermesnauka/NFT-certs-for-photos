using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;

namespace NftCerts.Certificates;

// Mirrors app01's Certificate JPA entity: a successfully minted on-chain NFT. The Artwork is
// always loaded eagerly since the DTO mapper reads its fields.
public class Certificate
{
    [Key]
    [DatabaseGenerated(DatabaseGeneratedOption.None)]  // tokenId comes from the chain
    public long TokenId { get; set; }
    public string ArtworkId { get; set; } = "";
    public Artwork Artwork { get; set; } = null!;
    public string ContentHashHex { get; set; } = "";
    public string ContractAddress { get; set; } = "";
    public string TxHash { get; set; } = "";
    public string OwnerAddress { get; set; } = "";
    public int RoyaltyPercentageBps { get; set; }
    public string RoyaltyRecipient { get; set; } = "";
    public string MintedAt { get; set; } = "";  // ISO-8601 instant
}
