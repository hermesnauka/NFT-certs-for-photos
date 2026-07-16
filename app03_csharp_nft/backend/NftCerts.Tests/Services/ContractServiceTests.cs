using NftCerts.Blockchain;
using NftCerts.Errors;

namespace NftCerts.Tests.Services;

// ContractService.MintCertificate/TokenContentHash themselves require a live JSON-RPC endpoint and
// are exercised by the manual end-to-end smoke test (docs/sdlc/08-test-plan.md §4), consistent with
// NFR-4 (automated tests must not require live network access). The one pure, deterministic piece
// of logic — validating/parsing the content hash hex string — is unit-testable directly.
public class ContractServiceTests
{
    [Fact]
    public void HexToBytes32_ValidHashWithPrefix_ReturnsThirtyTwoBytes()
    {
        string hex = "0x" + new string('a', 64);

        byte[] bytes = ContractService.HexToBytes32(hex);

        Assert.Equal(32, bytes.Length);
    }

    [Fact]
    public void HexToBytes32_ValidHashWithoutPrefix_ReturnsThirtyTwoBytes()
    {
        byte[] bytes = ContractService.HexToBytes32(new string('b', 64));

        Assert.Equal(32, bytes.Length);
    }

    [Theory]
    [InlineData("0x1234")]
    [InlineData("")]
    public void HexToBytes32_WrongLength_ThrowsMintingException(string hex)
    {
        Assert.Throws<MintingException>(() => ContractService.HexToBytes32(hex));
    }
}
