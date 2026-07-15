package com.devpowers.nftcerts.api;

import com.devpowers.nftcerts.certificate.Artwork;
import com.devpowers.nftcerts.certificate.ArtworkStatus;
import com.devpowers.nftcerts.certificate.Certificate;
import com.devpowers.nftcerts.certificate.CertificateService;
import com.devpowers.nftcerts.config.ExplorerLinkProperties;
import com.devpowers.nftcerts.error.CertificateNotFoundException;
import com.devpowers.nftcerts.pdf.CertificatePdfService;
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
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.content;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@WebMvcTest(CertificateController.class)
@Import({CertificateDtoMapper.class, ExplorerLinkProperties.class})
@ActiveProfiles("test")
class CertificateControllerTest {

    @Autowired
    private MockMvc mockMvc;

    @MockBean
    private CertificateService certificateService;

    @MockBean
    private CertificatePdfService certificatePdfService;

    @Test
    void getCertificateReturnsExpectedShape() throws Exception {
        Certificate certificate = buildCertificate();
        when(certificateService.getCertificate(42L)).thenReturn(certificate);

        mockMvc.perform(get("/api/certificates/42"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.tokenId").value(42))
                .andExpect(jsonPath("$.contentHashHex").value("a".repeat(64)))
                .andExpect(jsonPath("$.ownerAddress").value("0xowner"))
                .andExpect(jsonPath("$.openSeaUrl").exists())
                .andExpect(jsonPath("$.raribleUrl").exists())
                .andExpect(jsonPath("$.etherscanUrl").exists());
    }

    @Test
    void getCertificateReturns404WhenUnknown() throws Exception {
        when(certificateService.getCertificate(999L)).thenThrow(new CertificateNotFoundException(999L));

        mockMvc.perform(get("/api/certificates/999"))
                .andExpect(status().isNotFound())
                .andExpect(jsonPath("$.status").value(404));
    }

    @Test
    void getCertificatePdfReturnsPdfContentType() throws Exception {
        Certificate certificate = buildCertificate();
        when(certificateService.getCertificate(42L)).thenReturn(certificate);
        when(certificatePdfService.generate(any(), any(), any(), any())).thenReturn(new byte[]{1, 2, 3});

        mockMvc.perform(get("/api/certificates/42/pdf"))
                .andExpect(status().isOk())
                .andExpect(content().contentType(MediaType.APPLICATION_PDF));
    }

    private Certificate buildCertificate() {
        Artwork artwork = new Artwork(UUID.randomUUID(), UUID.randomUUID(), "photo.jpg", "a".repeat(64), "Sunset",
                "desc", "photograph", 2024, 750, "0xowner", "did:key:zArtist", ArtworkStatus.MINTED, Instant.now());
        artwork.setImageIpfsUri("ipfs://image-cid");
        artwork.setMetadataIpfsUri("ipfs://metadata-cid");

        return new Certificate(42L, artwork, "a".repeat(64), "0xcontract", "0xtxhash", "0xowner", 750, "0xowner", Instant.now());
    }
}
