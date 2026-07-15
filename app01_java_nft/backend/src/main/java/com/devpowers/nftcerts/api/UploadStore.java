package com.devpowers.nftcerts.api;

import com.devpowers.nftcerts.error.FileNotFoundInUploadStoreException;
import org.springframework.stereotype.Component;

import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Simple in-memory store for uploaded (hashed/watermarked) file bytes, retrievable later by
 * {@code fileId} when creating an artwork. H2 is not used for blob storage per the design spec.
 */
@Component
public class UploadStore {

    private final Map<UUID, StoredUpload> uploads = new ConcurrentHashMap<>();

    public UUID store(StoredUpload upload) {
        UUID fileId = UUID.randomUUID();
        uploads.put(fileId, upload);
        return fileId;
    }

    public StoredUpload get(UUID fileId) {
        StoredUpload upload = uploads.get(fileId);
        if (upload == null) {
            throw new FileNotFoundInUploadStoreException(fileId);
        }
        return upload;
    }
}
