package com.gandarych.nftcerts.api;

import com.gandarych.nftcerts.certificate.Artwork;
import com.gandarych.nftcerts.certificate.ArtworkStatus;
import com.gandarych.nftcerts.certificate.Certificate;
import com.gandarych.nftcerts.certificate.CertificateService;
import com.gandarych.nftcerts.config.ExplorerLinkProperties;
import com.gandarych.nftcerts.error.ArtworkNotFoundException;
import com.gandarych.nftcerts.error.DuplicateContentHashException;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Import;
import org.springframework.http.MediaType;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;

import java.time.Instant;
import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@WebMvcTest(ArtworkController.class)
@Import({CertificateDtoMapper.class, ExplorerLinkProperties.class})
@ActiveProfiles("test")
class ArtworkControllerTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private ObjectMapper objectMapper;

    @MockBean
    private UploadStore uploadStore;

    @MockBean
    private CertificateService certificateService;

    @Test
    void createArtworkReturns201WithPinnedStatus() throws Exception {
        UUID fileId = UUID.randomUUID();
        StoredUpload upload = new StoredUpload("photo.jpg", "image/jpeg", "a".repeat(64), "bytes".getBytes());
        when(uploadStore.get(fileId)).thenReturn(upload);

        Artwork artwork = new Artwork(UUID.randomUUID(), fileId, "photo.jpg", "a".repeat(64), "Sunset",
                "desc", "photograph", 2024, 750, "0xabc", "did:key:zArtist", ArtworkStatus.PINNED, Instant.now());
        artwork.setImageIpfsUri("ipfs://image-cid");
        artwork.setMetadataIpfsUri("ipfs://metadata-cid");

        when(certificateService.createArtwork(any(), anyString(), any(), any(), any(), any(), anyString(), any()))
                .thenReturn(artwork);

        CreateArtworkRequest request = new CreateArtworkRequest(
                fileId, "Sunset", "desc", "photograph", 2024, 750, "0xabc", "did:key:zArtist");

        mockMvc.perform(post("/api/artworks")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andExpect(status().isCreated())
                .andExpect(jsonPath("$.artworkId").value(artwork.getId().toString()))
                .andExpect(jsonPath("$.sha256Hash").value("a".repeat(64)))
                .andExpect(jsonPath("$.imageIpfsUri").value("ipfs://image-cid"))
                .andExpect(jsonPath("$.metadataIpfsUri").value("ipfs://metadata-cid"))
                .andExpect(jsonPath("$.status").value("PINNED"));
    }

    @Test
    void mintReturns409OnDuplicateContentHash() throws Exception {
        UUID artworkId = UUID.randomUUID();
        when(certificateService.mintCertificate(eq(artworkId), anyString()))
                .thenThrow(new DuplicateContentHashException("a".repeat(64)));

        MintRequest request = new MintRequest("0xrecipient000000000000000000000000000001");

        mockMvc.perform(post("/api/artworks/" + artworkId + "/mint")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.status").value(409));
    }

    @Test
    void mintReturns404ForUnknownArtwork() throws Exception {
        UUID artworkId = UUID.randomUUID();
        when(certificateService.mintCertificate(eq(artworkId), anyString()))
                .thenThrow(new ArtworkNotFoundException(artworkId));

        MintRequest request = new MintRequest("0xrecipient000000000000000000000000000001");

        mockMvc.perform(post("/api/artworks/" + artworkId + "/mint")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andExpect(status().isNotFound())
                .andExpect(jsonPath("$.status").value(404));
    }
}
