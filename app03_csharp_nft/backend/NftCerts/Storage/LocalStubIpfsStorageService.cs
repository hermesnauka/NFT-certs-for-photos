using System.Text;
using System.Text.Json;
using NftCerts.Hashing;

namespace NftCerts.Storage;

// Offline stand-in for Pinata: writes the pinned bytes to a local temp directory and returns a
// deterministic fake "ipfs://stub-<sha256>" URI. Mirrors app01/app02's LocalStubIpfsStorageService.
public class LocalStubIpfsStorageService : IIpfsStorageService
{
    private readonly Sha256HashingService _hashing = new();
    private readonly string _storageDir = Path.Combine(Path.GetTempPath(), "local-stub-ipfs");

    public LocalStubIpfsStorageService() => Directory.CreateDirectory(_storageDir);

    public PinResult PinFile(byte[] content, string filename, string contentType) => StoreAndBuildResult(content);

    public PinResult PinJson(JsonElement jsonPayload, string name) =>
        StoreAndBuildResult(Encoding.UTF8.GetBytes(jsonPayload.GetRawText()));

    private PinResult StoreAndBuildResult(byte[] content)
    {
        string cid = "stub-" + _hashing.Sha256Hex(content);
        File.WriteAllBytes(Path.Combine(_storageDir, cid), content);
        return PinResult.Of(cid);
    }
}
