package com.devpowers.nftcerts.api;

import com.devpowers.nftcerts.error.EmptyFileException;
import com.devpowers.nftcerts.error.UnsupportedFileTypeException;
import com.devpowers.nftcerts.hashing.Sha256HashingService;
import com.devpowers.nftcerts.watermark.MetadataWatermarkService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.util.Set;
import java.util.UUID;

/**
 * POST /api/uploads — accepts a photograph, computes its SHA-256 "digital fingerprint", embeds a
 * best-effort EXIF watermark, and stores the (possibly watermarked) bytes for later use when
 * creating an artwork.
 */
@RestController
public class UploadController {

    private static final Set<String> SUPPORTED_CONTENT_TYPES = Set.of("image/jpeg", "image/png", "image/webp");

    private final Sha256HashingService hashingService;
    private final MetadataWatermarkService watermarkService;
    private final UploadStore uploadStore;

    public UploadController(Sha256HashingService hashingService, MetadataWatermarkService watermarkService,
                             UploadStore uploadStore) {
        this.hashingService = hashingService;
        this.watermarkService = watermarkService;
        this.uploadStore = uploadStore;
    }

    @PostMapping(value = "/api/uploads", consumes = "multipart/form-data")
    public ResponseEntity<UploadResponse> upload(@RequestParam("file") MultipartFile file) {
        if (file.isEmpty()) {
            throw new EmptyFileException();
        }
        String contentType = file.getContentType();
        if (contentType == null || !SUPPORTED_CONTENT_TYPES.contains(contentType.toLowerCase())) {
            throw new UnsupportedFileTypeException(contentType);
        }

        byte[] originalContent;
        try {
            originalContent = file.getBytes();
        } catch (IOException e) {
            throw new UncheckedIOException("Failed to read uploaded file", e);
        }

        String sha256Hash = hashingService.sha256Hex(originalContent);
        // Watermark payload references the hash of the *original* content; the artist DID is not
        // yet known at upload time (it is supplied later on /api/artworks), so an empty marker is
        // used here and the definitive DID-linked watermark/hash pairing lives in the metadata JSON
        // pinned to IPFS during artwork creation.
        byte[] watermarkedContent = watermarkService.watermark(originalContent, contentType, sha256Hash, "");

        StoredUpload storedUpload = new StoredUpload(file.getOriginalFilename(), contentType, sha256Hash, watermarkedContent);
        UUID fileId = uploadStore.store(storedUpload);

        UploadResponse response = new UploadResponse(fileId, file.getOriginalFilename(), sha256Hash, watermarkedContent.length);
        return ResponseEntity.ok(response);
    }
}
