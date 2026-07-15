package com.gandarych.nftcerts.storage;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HexFormat;

/**
 * Local, no-network stand-in for {@link IpfsStorageService} used only under the {@code test}
 * profile. Writes content to a local temp directory and returns a deterministic fake
 * {@code ipfs://stub-<sha256-of-content>} URI, so tests never depend on Pinata availability.
 */
@Service
@Profile("test")
public class LocalStubIpfsStorageService implements IpfsStorageService {

    private final ObjectMapper objectMapper;
    private final Path storageDir;

    public LocalStubIpfsStorageService(ObjectMapper objectMapper) {
        this.objectMapper = objectMapper;
        try {
            this.storageDir = Files.createTempDirectory("local-stub-ipfs");
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    @Override
    public PinResult pinFile(byte[] content, String filename, String contentType) {
        return storeAndBuildResult(content);
    }

    @Override
    public PinResult pinJson(Object jsonPayload, String name) {
        try {
            byte[] content = objectMapper.writeValueAsBytes(jsonPayload);
            return storeAndBuildResult(content);
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
    }

    private PinResult storeAndBuildResult(byte[] content) {
        String cid = "stub-" + sha256Hex(content);
        try {
            Files.write(storageDir.resolve(cid), content);
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
        return PinResult.of(cid);
    }

    private String sha256Hex(byte[] content) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            return HexFormat.of().formatHex(digest.digest(content));
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalStateException("SHA-256 algorithm not available", e);
        }
    }
}
