package com.gandarych.nftcerts.hashing;

import org.springframework.stereotype.Service;

import java.io.IOException;
import java.io.InputStream;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HexFormat;

/**
 * Computes the SHA-256 "digital fingerprint" of an uploaded artwork file, streaming the content
 * through a {@link DigestInputStream} rather than loading the whole file into memory.
 */
@Service
public class Sha256HashingService {

    private static final String ALGORITHM = "SHA-256";

    /**
     * Streams the given input and returns its lowercase hex-encoded SHA-256 digest.
     *
     * @param inputStream the file content; the caller owns and closes this stream.
     */
    public String sha256Hex(InputStream inputStream) {
        try {
            MessageDigest digest = MessageDigest.getInstance(ALGORITHM);
            try (DigestInputStream digestInputStream = new DigestInputStream(inputStream, digest)) {
                byte[] buffer = new byte[8192];
                while (digestInputStream.read(buffer) != -1) {
                    // discard - DigestInputStream updates the digest as a side effect of reading
                }
            }
            return HexFormat.of().formatHex(digest.digest());
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalStateException("SHA-256 algorithm not available", e);
        } catch (IOException e) {
            throw new IllegalStateException("Failed to read file content for hashing", e);
        }
    }

    /**
     * Convenience overload for already-loaded byte arrays (e.g. after watermarking).
     */
    public String sha256Hex(byte[] content) {
        return sha256Hex(new java.io.ByteArrayInputStream(content));
    }
}
