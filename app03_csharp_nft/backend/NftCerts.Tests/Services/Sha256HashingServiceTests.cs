using System.Text;
using NftCerts.Hashing;

namespace NftCerts.Tests.Services;

public class Sha256HashingServiceTests
{
    private readonly Sha256HashingService _service = new();

    [Fact]
    public void Sha256Hex_OfEmptyInput_MatchesKnownVector()
    {
        Assert.Equal("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b85",
                      _service.Sha256Hex([]));
    }

    [Fact]
    public void Sha256Hex_OfKnownInput_MatchesKnownVector()
    {
        byte[] content = Encoding.ASCII.GetBytes("abc");
        Assert.Equal("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
                      _service.Sha256Hex(content));
    }

    [Fact]
    public void Sha256Hex_StreamOverload_MatchesByteArrayOverload()
    {
        byte[] content = Encoding.UTF8.GetBytes("streaming hash should match byte[] hash");
        using var stream = new MemoryStream(content);

        Assert.Equal(_service.Sha256Hex(content), _service.Sha256Hex(stream));
    }

    [Fact]
    public void Sha256Hex_IsDeterministicAndCaseInsensitiveHex()
    {
        byte[] content = [1, 2, 3, 4, 5];
        string hash = _service.Sha256Hex(content);

        Assert.Equal(64, hash.Length);
        Assert.Equal(hash, hash.ToLowerInvariant());
        Assert.Equal(hash, _service.Sha256Hex(content));
    }
}
