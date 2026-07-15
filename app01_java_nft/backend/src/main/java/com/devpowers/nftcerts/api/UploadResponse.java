package com.devpowers.nftcerts.api;

import java.util.UUID;

public record UploadResponse(UUID fileId, String originalFilename, String sha256Hash, long sizeBytes) {
}
