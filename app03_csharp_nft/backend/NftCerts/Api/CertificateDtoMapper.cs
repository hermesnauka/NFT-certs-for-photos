using NftCerts.Certificates;
using NftCerts.Config;

namespace NftCerts.Api;

// Maps a Certificate to the API DTO shared with app01/app02, including marketplace deep links.
public class CertificateDtoMapper(ExplorerLinkProperties explorer)
{
    public object ToDto(Certificate certificate) => new
    {
        tokenId = certificate.TokenId,
        artworkId = certificate.Artwork.Id,
        title = certificate.Artwork.Title,
        contentHashHex = certificate.ContentHashHex,
        contractAddress = certificate.ContractAddress,
        txHash = certificate.TxHash,
        ownerAddress = certificate.OwnerAddress,
        royaltyPercentageBps = certificate.RoyaltyPercentageBps,
        imageIpfsUri = certificate.Artwork.ImageIpfsUri,
        metadataIpfsUri = certificate.Artwork.MetadataIpfsUri,
        etherscanUrl = BuildEtherscanUrl(certificate),
        openSeaUrl = BuildOpenSeaUrl(certificate),
        raribleUrl = BuildRaribleUrl(certificate),
    };

    public string BuildEtherscanUrl(Certificate certificate) =>
        explorer.EtherscanBaseUrl + certificate.TxHash;

    public string BuildOpenSeaUrl(Certificate certificate) =>
        $"{explorer.OpenseaBaseUrl}{certificate.ContractAddress}/{certificate.TokenId}";

    public string BuildRaribleUrl(Certificate certificate) =>
        $"{explorer.RaribleBaseUrl}{certificate.ContractAddress}:{certificate.TokenId}";
}
