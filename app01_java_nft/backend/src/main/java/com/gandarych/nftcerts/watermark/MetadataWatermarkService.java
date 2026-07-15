package com.gandarych.nftcerts.watermark;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.apache.commons.imaging.Imaging;
import org.apache.commons.imaging.common.ImageMetadata;
import org.apache.commons.imaging.formats.jpeg.JpegImageMetadata;
import org.apache.commons.imaging.formats.jpeg.exif.ExifRewriter;
import org.apache.commons.imaging.formats.tiff.TiffImageMetadata;
import org.apache.commons.imaging.formats.tiff.constants.TiffTagConstants;
import org.apache.commons.imaging.formats.tiff.write.TiffOutputDirectory;
import org.apache.commons.imaging.formats.tiff.write.TiffOutputSet;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Embeds a small JSON payload linking a photograph to its content hash and the artist's DID
 * directly into the file's EXIF metadata, providing an additional (non-cryptographic) provenance
 * signal alongside the on-chain/IPFS record.
 *
 * <p>Implementation note: Apache Commons Imaging exposes {@code EXIF_TAG_USER_COMMENT} as a
 * {@code TagInfoGpsText} field, which requires the EXIF "character code" prefix framing rather
 * than a plain string. To keep this straightforward and robust across the resolved library
 * version (1.0.0-alpha5), this implementation instead writes the JSON payload into the root
 * IFD0 {@code ImageDescription} tag ({@link TiffTagConstants#TIFF_TAG_IMAGE_DESCRIPTION}), which
 * is a plain ASCII field and is well supported for lossless rewriting via {@link ExifRewriter}.
 */
@Service
public class MetadataWatermarkService {

    private static final Logger log = LoggerFactory.getLogger(MetadataWatermarkService.class);
    private static final String JPEG_CONTENT_TYPE = "image/jpeg";

    private final ObjectMapper objectMapper;

    public MetadataWatermarkService(ObjectMapper objectMapper) {
        this.objectMapper = objectMapper;
    }

    /**
     * Embeds {@code {"contentHash":"<hex>","artistDid":"<did>"}} into the EXIF ImageDescription
     * field of a JPEG. For any other content type, the original bytes are returned unchanged and
     * a warning is logged (documented behavior, not a bug).
     */
    public byte[] watermark(byte[] originalContent, String contentType, String contentHash, String artistDid) {
        if (!JPEG_CONTENT_TYPE.equalsIgnoreCase(contentType)) {
            log.warn("Skipping EXIF watermark embedding for unsupported content type '{}': only {} is supported",
                    contentType, JPEG_CONTENT_TYPE);
            return originalContent;
        }

        try {
            String payloadJson = buildPayloadJson(contentHash, artistDid);

            TiffOutputSet outputSet = resolveOutputSet(originalContent);
            TiffOutputDirectory rootDirectory = outputSet.getOrCreateRootDirectory();
            rootDirectory.removeField(TiffTagConstants.TIFF_TAG_IMAGE_DESCRIPTION);
            rootDirectory.add(TiffTagConstants.TIFF_TAG_IMAGE_DESCRIPTION, payloadJson);

            ByteArrayOutputStream output = new ByteArrayOutputStream();
            new ExifRewriter().updateExifMetadataLossless(originalContent, output, outputSet);
            return output.toByteArray();
        } catch (IOException e) {
            log.warn("Failed to embed EXIF watermark, returning original file content unchanged", e);
            return originalContent;
        }
    }

    private String buildPayloadJson(String contentHash, String artistDid) {
        Map<String, String> payload = new LinkedHashMap<>();
        payload.put("contentHash", contentHash);
        payload.put("artistDid", artistDid);
        try {
            return objectMapper.writeValueAsString(payload);
        } catch (IOException e) {
            throw new IllegalStateException("Failed to serialize watermark payload", e);
        }
    }

    private TiffOutputSet resolveOutputSet(byte[] originalContent) throws IOException {
        ImageMetadata metadata = Imaging.getMetadata(originalContent);
        if (metadata instanceof JpegImageMetadata jpegImageMetadata) {
            TiffImageMetadata exif = jpegImageMetadata.getExif();
            if (exif != null) {
                TiffOutputSet existing = exif.getOutputSet();
                if (existing != null) {
                    return existing;
                }
            }
        }
        return new TiffOutputSet();
    }
}
