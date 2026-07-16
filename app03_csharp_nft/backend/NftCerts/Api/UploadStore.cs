using System.Collections.Concurrent;
using NftCerts.Certificates;
using NftCerts.Errors;

namespace NftCerts.Api;

// Simple in-memory store for uploaded (hashed/watermarked) file bytes, retrievable later by
// fileId when creating an artwork. Mirrors app01/app02's UploadStore.
public class UploadStore
{
    private readonly ConcurrentDictionary<string, StoredUpload> _uploads = new();

    public string Store(StoredUpload upload)
    {
        string fileId = Guid.NewGuid().ToString();
        _uploads[fileId] = upload;
        return fileId;
    }

    public StoredUpload Get(string fileId) =>
        _uploads.TryGetValue(fileId, out var upload) ? upload : throw new FileNotFoundInUploadStoreException(fileId);
}
