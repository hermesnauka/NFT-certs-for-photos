using System.Security.Cryptography;

namespace NftCerts.Hashing;

// Streaming SHA-256 hashing of uploaded files (NFR-1). Mirrors app01's HashingService.
public class Sha256HashingService
{
    public string Sha256Hex(byte[] content) => Convert.ToHexStringLower(SHA256.HashData(content));

    public string Sha256Hex(Stream stream) => Convert.ToHexStringLower(SHA256.HashData(stream));
}
