package com.gandarych.nftcerts.api;

import com.gandarych.nftcerts.certificate.Certificate;
import com.gandarych.nftcerts.certificate.CertificateService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

/** GET /api/artists/{walletAddress}/dashboard — an artist's certificates and royalty overview. */
@RestController
public class ArtistDashboardController {

    private final CertificateService certificateService;
    private final CertificateDtoMapper certificateDtoMapper;

    public ArtistDashboardController(CertificateService certificateService, CertificateDtoMapper certificateDtoMapper) {
        this.certificateService = certificateService;
        this.certificateDtoMapper = certificateDtoMapper;
    }

    @GetMapping("/api/artists/{walletAddress}/dashboard")
    public ResponseEntity<ArtistDashboardResponse> dashboard(@PathVariable String walletAddress) {
        List<Certificate> certificates = certificateService.getCertificatesForWallet(walletAddress);
        List<CertificateDto> dtos = certificates.stream().map(certificateDtoMapper::toDto).toList();
        return ResponseEntity.ok(new ArtistDashboardResponse(walletAddress, dtos, dtos.size()));
    }
}
