using System.Text;
using System.Text.Json;
using NftCerts.Hashing;
using NftCerts.Storage;

namespace NftCerts.Tests.Services;

public class LocalStubIpfsStorageServiceTests
{
    private readonly LocalStubIpfsStorageService _service = new();
    private readonly Sha256HashingService _hashing = new();

    [Fact]
    public void PinFile_ReturnsDeterministicStubUriBasedOnContentHash()
    {
        byte[] content = Encoding.UTF8.GetBytes("hello world");
        string expectedCid = "stub-" + _hashing.Sha256Hex(content);

        PinResult result = _service.PinFile(content, "hello.txt", "text/plain");

        Assert.Equal(expectedCid, result.Cid);
        Assert.Equal($"ipfs://{expectedCid}", result.Uri);
    }

    [Fact]
    public void PinJson_ReturnsDeterministicStubUriBasedOnSerializedContent()
    {
        JsonElement payload = JsonSerializer.SerializeToElement(new { title = "Lighthouse at Dusk" });
        byte[] expectedBytes = Encoding.UTF8.GetBytes(payload.GetRawText());
        string expectedCid = "stub-" + _hashing.Sha256Hex(expectedBytes);

        PinResult result = _service.PinJson(payload, "metadata");

        Assert.Equal(expectedCid, result.Cid);
    }

    [Fact]
    public void PinFile_SameContentTwice_ReturnsSameCid()
    {
        byte[] content = "same bytes"u8.ToArray();

        PinResult first = _service.PinFile(content, "a.txt", "text/plain");
        PinResult second = _service.PinFile(content, "b.txt", "text/plain");

        Assert.Equal(first.Cid, second.Cid);
    }
}
