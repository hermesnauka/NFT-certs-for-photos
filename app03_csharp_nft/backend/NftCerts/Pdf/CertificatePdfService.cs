using NftCerts.Certificates;
using QuestPDF.Fluent;
using QuestPDF.Helpers;
using QuestPDF.Infrastructure;

namespace NftCerts.Pdf;

// Renders a one-page, downloadable digital certificate of authenticity for a minted NFT
// certificate. QuestPDF here; app01 used OpenPDF, app02 libharu.
public class CertificatePdfService
{
    static CertificatePdfService() => QuestPDF.Settings.License = LicenseType.Community;

    public byte[] Generate(Certificate certificate, string etherscanUrl, string openSeaUrl, string raribleUrl)
    {
        var rows = new (string Label, string Value)[]
        {
            ("Token ID", certificate.TokenId.ToString()),
            ("Artist Wallet", certificate.OwnerAddress),
            ("Artist DID", certificate.Artwork.ArtistDid),
            ("Content Hash (SHA-256)", certificate.ContentHashHex),
            ("Medium", certificate.Artwork.Medium),
            ("Year Created", certificate.Artwork.YearCreated?.ToString() ?? "-"),
            ("Royalty", $"{certificate.RoyaltyPercentageBps / 100.0:0.##}%"),
            ("Royalty Recipient", certificate.RoyaltyRecipient),
            ("Contract Address", certificate.ContractAddress),
            ("Image (IPFS)", certificate.Artwork.ImageIpfsUri),
            ("Metadata (IPFS)", certificate.Artwork.MetadataIpfsUri),
            ("Transaction", certificate.TxHash),
            ("Minted At", certificate.MintedAt),
            ("Etherscan", etherscanUrl),
            ("OpenSea", openSeaUrl),
            ("Rarible", raribleUrl),
        };

        return Document.Create(container =>
        {
            container.Page(page =>
            {
                page.Size(PageSizes.A4);
                page.Margin(40);
                page.Content().Column(column =>
                {
                    column.Item().Text("Certificate of Authenticity").FontSize(24).Bold();
                    column.Item().PaddingBottom(8)
                        .Text(NullToDash(certificate.Artwork.Title)).FontSize(16).Italic();
                    foreach (var (label, value) in rows)
                    {
                        column.Item().PaddingTop(6).Row(row =>
                        {
                            row.ConstantItem(170).Text(label).SemiBold().FontSize(9);
                            row.RelativeItem().Text(NullToDash(value)).FontSize(9);
                        });
                    }
                    column.Item().PaddingTop(20).Text(
                        "This certificate attests that the photographic work identified by the SHA-256 content " +
                        "hash above has been registered as an ERC-721 token with EIP-2981 royalty information " +
                        "on the blockchain identified by the contract address above.").FontSize(8).Light();
                });
            });
        }).GeneratePdf();
    }

    private static string NullToDash(string? value) => string.IsNullOrEmpty(value) ? "-" : value;
}
