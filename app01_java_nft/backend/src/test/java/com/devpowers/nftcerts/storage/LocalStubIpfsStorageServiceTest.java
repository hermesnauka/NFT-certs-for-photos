package com.devpowers.nftcerts.storage;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.Test;

import java.nio.charset.StandardCharsets;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertTrue;

class LocalStubIpfsStorageServiceTest {

    private final LocalStubIpfsStorageService service = new LocalStubIpfsStorageService(new ObjectMapper());

    @Test
    void pinFileReturnsDeterministicStubCidAndIpfsUri() {
        byte[] content = "photo bytes".getBytes(StandardCharsets.UTF_8);

        PinResult first = service.pinFile(content, "photo.jpg", "image/jpeg");
        PinResult second = service.pinFile(content, "photo.jpg", "image/jpeg");

        assertEquals(first.cid(), second.cid());
        assertTrue(first.cid().startsWith("stub-"));
        assertEquals("ipfs://" + first.cid(), first.uri());
    }

    @Test
    void pinFileProducesDifferentCidsForDifferentContent() {
        PinResult a = service.pinFile("content-a".getBytes(StandardCharsets.UTF_8), "a.jpg", "image/jpeg");
        PinResult b = service.pinFile("content-b".getBytes(StandardCharsets.UTF_8), "b.jpg", "image/jpeg");

        assertNotNull(a.cid());
        assertNotNull(b.cid());
        assertTrue(!a.cid().equals(b.cid()));
    }

    @Test
    void pinJsonReturnsStubUri() {
        PinResult result = service.pinJson(Map.of("title", "Sunset"), "sunset-metadata");

        assertTrue(result.cid().startsWith("stub-"));
        assertEquals("ipfs://" + result.cid(), result.uri());
    }
}
