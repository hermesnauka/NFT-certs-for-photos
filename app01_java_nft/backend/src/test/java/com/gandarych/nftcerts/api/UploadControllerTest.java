package com.gandarych.nftcerts.api;

import com.gandarych.nftcerts.hashing.Sha256HashingService;
import com.gandarych.nftcerts.watermark.MetadataWatermarkService;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockMultipartFile;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.multipart;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@WebMvcTest(UploadController.class)
@ActiveProfiles("test")
class UploadControllerTest {

    @Autowired
    private MockMvc mockMvc;

    @MockBean
    private Sha256HashingService hashingService;

    @MockBean
    private MetadataWatermarkService watermarkService;

    @MockBean
    private UploadStore uploadStore;

    @Test
    void uploadReturnsFileIdHashAndSize() throws Exception {
        byte[] content = "fake-jpeg-bytes".getBytes();
        MockMultipartFile file = new MockMultipartFile("file", "lighthouse-at-dusk.jpg", "image/jpeg", content);

        when(hashingService.sha256Hex(any(byte[].class))).thenReturn("a".repeat(64));
        when(watermarkService.watermark(any(byte[].class), anyString(), anyString(), anyString())).thenReturn(content);
        when(uploadStore.store(any())).thenReturn(java.util.UUID.randomUUID());

        mockMvc.perform(multipart("/api/uploads").file(file))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.fileId").exists())
                .andExpect(jsonPath("$.originalFilename").value("lighthouse-at-dusk.jpg"))
                .andExpect(jsonPath("$.sha256Hash").value("a".repeat(64)))
                .andExpect(jsonPath("$.sizeBytes").value(content.length));
    }

    @Test
    void uploadRejectsEmptyFileWith400() throws Exception {
        MockMultipartFile emptyFile = new MockMultipartFile("file", "empty.jpg", "image/jpeg", new byte[0]);

        mockMvc.perform(multipart("/api/uploads").file(emptyFile))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.status").value(400));
    }

    @Test
    void uploadRejectsUnsupportedContentTypeWith400() throws Exception {
        MockMultipartFile textFile = new MockMultipartFile("file", "notes.txt", "text/plain", "hello".getBytes());

        mockMvc.perform(multipart("/api/uploads").file(textFile))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.status").value(400));
    }
}
