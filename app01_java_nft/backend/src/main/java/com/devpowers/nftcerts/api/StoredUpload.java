package com.devpowers.nftcerts.api;

/** In-memory record of a hashed (and possibly watermarked) uploaded file, keyed by fileId. */
public record StoredUpload(String originalFilename, String contentType, String sha256Hash, byte[] content) {
}
