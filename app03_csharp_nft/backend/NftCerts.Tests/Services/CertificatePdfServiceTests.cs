using System.Text;
using NftCerts.Certificates;
using NftCerts.Pdf;

namespace NftCerts.Tests.Services;

public class CertificatePdfServiceTests
{
    private readonly CertificatePdfService _service = new();

    private static Certificate SampleCertificate() => new()
    {
        TokenId = 42,
        ArtworkId = "artwork-1",
        Artwork = new Artwork
        {
            Id = "artwork-1",
            Title = "Lighthouse at Dusk",
            ArtistDid = "did:key:z6Mk...",
            Medium = "Archival pigment print",
            YearCreated = 2024,
            ImageIpfsUri = "ipfs://image-cid",
            MetadataIpfsUri = "ipfs://metadata-cid",
        },
        ContentHashHex = "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a0",
        ContractAddress = "0x5FbDB2315678afecb367f032d93F642f64180aa",
        TxHash = "0xdeadbeef",
        OwnerAddress = "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567",
        RoyaltyPercentageBps = 750,
        RoyaltyRecipient = "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567",
        MintedAt = "2026-07-15T10:15:30Z",
    };

    [Fact]
    public void Generate_ReturnsNonEmptyValidPdfBytes()
    {
        byte[] pdf = _service.Generate(SampleCertificate(), "http://etherscan/tx/0xdeadbeef",
                                        "http://opensea/x/42", "http://rarible/x:42");

        Assert.NotEmpty(pdf);
        string header = Encoding.ASCII.GetString(pdf, 0, 5);
        Assert.Equal("%PDF-", header);
        Assert.Contains("%%EOF", Encoding.ASCII.GetString(pdf));
    }
}
