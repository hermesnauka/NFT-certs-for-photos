using System.Text.Json;

namespace NftCerts.Storage;

public record PinResult(string Cid, string Uri)
{
    public static PinResult Of(string cid) => new(cid, $"ipfs://{cid}");
}

// Abstraction over decentralized (IPFS) storage pinning. Two implementations exist:
// PinataIpfsStorageService (real, used outside the mock storage profile) and
// LocalStubIpfsStorageService (local stub, wired only when APP_STORAGE_PROVIDER=mock).
public interface IIpfsStorageService
{
    // Pins raw file bytes (e.g. the source image) to IPFS.
    PinResult PinFile(byte[] content, string filename, string contentType);

    // Pins a JSON-serializable payload (e.g. NFT metadata) to IPFS.
    PinResult PinJson(JsonElement jsonPayload, string name);
}
