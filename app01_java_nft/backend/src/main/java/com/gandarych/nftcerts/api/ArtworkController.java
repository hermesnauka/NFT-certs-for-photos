package com.gandarych.nftcerts.api;

import com.gandarych.nftcerts.certificate.Artwork;
import com.gandarych.nftcerts.certificate.Certificate;
import com.gandarych.nftcerts.certificate.CertificateService;
import jakarta.validation.Valid;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.UUID;

/**
 * POST /api/artworks and POST /api/artworks/{artworkId}/mint — creates an artwork (hash + IPFS
 * pinning) from a previously-uploaded file, and mints its on-chain NFT certificate.
 */
@RestController
@RequestMapping("/api/artworks")
public class ArtworkController {

    private final UploadStore uploadStore;
    private final CertificateService certificateService;
    private final CertificateDtoMapper certificateDtoMapper;

    public ArtworkController(UploadStore uploadStore, CertificateService certificateService,
                              CertificateDtoMapper certificateDtoMapper) {
        this.uploadStore = uploadStore;
        this.certificateService = certificateService;
        this.certificateDtoMapper = certificateDtoMapper;
    }

    @PostMapping
    public ResponseEntity<ArtworkResponse> createArtwork(@Valid @RequestBody CreateArtworkRequest request) {
        StoredUpload upload = uploadStore.get(request.fileId());

        Artwork artwork = certificateService.createArtwork(
                upload,
                request.title(),
                request.description(),
                request.medium(),
                request.yearCreated(),
                request.royaltyPercentageBps(),
                request.artistWalletAddress(),
                request.artistDid());

        ArtworkResponse response = new ArtworkResponse(
                artwork.getId(),
                artwork.getSha256Hash(),
                artwork.getImageIpfsUri(),
                artwork.getMetadataIpfsUri(),
                artwork.getStatus());

        return ResponseEntity.status(HttpStatus.CREATED).body(response);
    }

    @PostMapping("/{artworkId}/mint")
    public ResponseEntity<CertificateDto> mint(@PathVariable UUID artworkId, @Valid @RequestBody MintRequest request) {
        Certificate certificate = certificateService.mintCertificate(artworkId, request.recipientAddress());
        return ResponseEntity.ok(certificateDtoMapper.toDto(certificate));
    }
}
