package com.gandarych.nftcerts.api;

import com.gandarych.nftcerts.certificate.Certificate;
import com.gandarych.nftcerts.certificate.CertificateService;
import com.gandarych.nftcerts.pdf.CertificatePdfService;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * GET /api/certificates/{tokenId} and GET /api/certificates/{tokenId}/pdf — read-only access to a
 * minted certificate's data and its downloadable PDF certificate of authenticity.
 */
@RestController
@RequestMapping("/api/certificates")
public class CertificateController {

    private final CertificateService certificateService;
    private final CertificateDtoMapper certificateDtoMapper;
    private final CertificatePdfService certificatePdfService;

    public CertificateController(CertificateService certificateService, CertificateDtoMapper certificateDtoMapper,
                                  CertificatePdfService certificatePdfService) {
        this.certificateService = certificateService;
        this.certificateDtoMapper = certificateDtoMapper;
        this.certificatePdfService = certificatePdfService;
    }

    @GetMapping("/{tokenId}")
    public ResponseEntity<CertificateDto> getCertificate(@PathVariable Long tokenId) {
        Certificate certificate = certificateService.getCertificate(tokenId);
        return ResponseEntity.ok(certificateDtoMapper.toDto(certificate));
    }

    @GetMapping("/{tokenId}/pdf")
    public ResponseEntity<byte[]> getCertificatePdf(@PathVariable Long tokenId) {
        Certificate certificate = certificateService.getCertificate(tokenId);
        byte[] pdf = certificatePdfService.generate(
                certificate,
                certificateDtoMapper.buildEtherscanUrl(certificate),
                certificateDtoMapper.buildOpenSeaUrl(certificate),
                certificateDtoMapper.buildRaribleUrl(certificate));

        return ResponseEntity.ok()
                .contentType(MediaType.APPLICATION_PDF)
                .header(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=\"certificate-" + tokenId + ".pdf\"")
                .body(pdf);
    }
}
