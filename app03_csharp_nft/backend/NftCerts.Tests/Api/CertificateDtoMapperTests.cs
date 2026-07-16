using System.Text.Json;
using NftCerts.Api;
using NftCerts.Certificates;
using NftCerts.Config;

namespace NftCerts.Tests.Api;

public class CertificateDtoMapperTests
{
    private readonly CertificateDtoMapper _mapper = new(new ExplorerLinkProperties
    {
        EtherscanBaseUrl = "http://localhost:8545/tx/",
        OpenseaBaseUrl = "https://testnets.opensea.io/assets/",
        RaribleBaseUrl = "https://rarible.com/token/",
    });

    private static Certificate SampleCertificate() => new()
    {
        TokenId = 42,
        ArtworkId = "artwork-1",
        Artwork = new Artwork { Id = "artwork-1", Title = "Lighthouse at Dusk", ImageIpfsUri = "ipfs://img",
                                 MetadataIpfsUri = "ipfs://meta" },
        ContentHashHex = "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a0",
        ContractAddress = "0x5FbDB2315678afecb367f032d93F642f64180aa",
        TxHash = "0xdeadbeef",
        OwnerAddress = "0xA1b2C3d4E5f60718293a4b5c6d7e8f901234567",
        RoyaltyPercentageBps = 750,
    };

    [Fact]
    public void BuildEtherscanUrl_AppendsTxHashToBase()
    {
        Assert.Equal("http://localhost:8545/tx/0xdeadbeef", _mapper.BuildEtherscanUrl(SampleCertificate()));
    }

    [Fact]
    public void BuildOpenSeaUrl_AppendsContractAndTokenId()
    {
        Assert.Equal("https://testnets.opensea.io/assets/0x5FbDB2315678afecb367f032d93F642f64180aa/42",
                      _mapper.BuildOpenSeaUrl(SampleCertificate()));
    }

    [Fact]
    public void BuildRaribleUrl_AppendsContractColonTokenId()
    {
        Assert.Equal("https://rarible.com/token/0x5FbDB2315678afecb367f032d93F642f64180aa:42",
                      _mapper.BuildRaribleUrl(SampleCertificate()));
    }

    [Fact]
    public void ToDto_IncludesAllDocumentedFields()
    {
        object dto = _mapper.ToDto(SampleCertificate());
        JsonElement json = JsonSerializer.SerializeToElement(dto);

        Assert.Equal(42, json.GetProperty("tokenId").GetInt64());
        Assert.Equal("artwork-1", json.GetProperty("artworkId").GetString());
        Assert.Equal("Lighthouse at Dusk", json.GetProperty("title").GetString());
        Assert.Equal(750, json.GetProperty("royaltyPercentageBps").GetInt32());
        Assert.Equal("ipfs://img", json.GetProperty("imageIpfsUri").GetString());
        Assert.Equal("ipfs://meta", json.GetProperty("metadataIpfsUri").GetString());
        Assert.True(json.TryGetProperty("etherscanUrl", out _));
        Assert.True(json.TryGetProperty("openSeaUrl", out _));
        Assert.True(json.TryGetProperty("raribleUrl", out _));
    }
}
