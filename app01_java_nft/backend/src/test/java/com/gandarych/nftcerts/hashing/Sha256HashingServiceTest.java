package com.gandarych.nftcerts.hashing;

import org.junit.jupiter.api.Test;

import java.io.ByteArrayInputStream;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.HexFormat;

import static org.junit.jupiter.api.Assertions.assertEquals;

class Sha256HashingServiceTest {

    private final Sha256HashingService service = new Sha256HashingService();

    @Test
    void computesKnownSha256HexForSimpleString() throws Exception {
        byte[] content = "hello world".getBytes(StandardCharsets.UTF_8);

        String actual = service.sha256Hex(new ByteArrayInputStream(content));

        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        String expected = HexFormat.of().formatHex(digest.digest(content));

        assertEquals(expected, actual);
        assertEquals(64, actual.length());
    }

    @Test
    void byteArrayOverloadMatchesStreamOverload() {
        byte[] content = "another payload".getBytes(StandardCharsets.UTF_8);

        String viaBytes = service.sha256Hex(content);
        String viaStream = service.sha256Hex(new ByteArrayInputStream(content));

        assertEquals(viaStream, viaBytes);
    }

    @Test
    void hashIsLowercaseHex() {
        String hash = service.sha256Hex("Test Content".getBytes(StandardCharsets.UTF_8));

        assertEquals(hash.toLowerCase(), hash);
        assertEquals(64, hash.length());
    }
}
