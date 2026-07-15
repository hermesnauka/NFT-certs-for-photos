package com.devpowers.nftcerts.certificate;

import com.devpowers.nftcerts.api.StoredUpload;
import com.devpowers.nftcerts.blockchain.ContractService;
import com.devpowers.nftcerts.blockchain.MintResult;
import com.devpowers.nftcerts.error.ArtworkNotFoundException;
import com.devpowers.nftcerts.error.ArtworkNotPinnedException;
import com.devpowers.nftcerts.error.CertificateNotFoundException;
import com.devpowers.nftcerts.error.DuplicateContentHashException;
import com.devpowers.nftcerts.error.NotVerifiedException;
import com.devpowers.nftcerts.error.RoyaltyOutOfRangeException;
import com.devpowers.nftcerts.identity.KycVerificationService;
import com.devpowers.nftcerts.storage.IpfsStorageService;
import com.devpowers.nftcerts.storage.PinResult;
import org.springframework.stereotype.Service;

import java.math.BigInteger;
import java.time.Instant;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * Orchestrates the artwork -> IPFS pinning -> on-chain minting -> Certificate persistence
 * pipeline described in the platform's SDLC design docs.
 */
@Service
public class CertificateService {

    private final ArtworkRepository artworkRepository;
    private final CertificateRepository certificateRepository;
    private final IpfsStorageService ipfsStorageService;
    private final ContractService contractService;
    private final KycVerificationService kycVerificationService;

    public CertificateService(ArtworkRepository artworkRepository, CertificateRepository certificateRepository,
                               IpfsStorageService ipfsStorageService, ContractService contractService,
                               KycVerificationService kycVerificationService) {
        this.artworkRepository = artworkRepository;
        this.certificateRepository = certificateRepository;
        this.ipfsStorageService = ipfsStorageService;
        this.contractService = contractService;
        this.kycVerificationService = kycVerificationService;
    }

    /**
     * Creates and persists an {@link Artwork} from a previously-uploaded, hashed file, then pins
     * both the image and its generated metadata JSON to IPFS.
     */
    public Artwork createArtwork(StoredUpload upload, String title, String description, String medium,
                                  Integer yearCreated, Integer royaltyPercentageBps, String artistWalletAddress,
                                  String artistDid) {
        if (royaltyPercentageBps == null || royaltyPercentageBps < 0 || royaltyPercentageBps > 10000) {
            throw new RoyaltyOutOfRangeException(royaltyPercentageBps);
        }

        Artwork artwork = new Artwork(
                UUID.randomUUID(),
                UUID.randomUUID(),
                upload.originalFilename(),
                upload.sha256Hash(),
                title,
                description,
                medium,
                yearCreated,
                royaltyPercentageBps,
                artistWalletAddress,
                artistDid,
                ArtworkStatus.UPLOADED,
                Instant.now());

        PinResult imagePin = ipfsStorageService.pinFile(upload.content(), upload.originalFilename(), upload.contentType());
        artwork.setImageIpfsUri(imagePin.uri());

        Map<String, Object> metadata = buildMetadataPayload(artwork, imagePin);
        PinResult metadataPin = ipfsStorageService.pinJson(metadata, artwork.getId() + "-metadata");
        artwork.setMetadataIpfsUri(metadataPin.uri());

        artwork.setStatus(ArtworkStatus.PINNED);

        return artworkRepository.save(artwork);
    }

    private Map<String, Object> buildMetadataPayload(Artwork artwork, PinResult imagePin) {
        Map<String, Object> metadata = new LinkedHashMap<>();
        metadata.put("title", artwork.getTitle());
        metadata.put("description", artwork.getDescription());
        metadata.put("medium", artwork.getMedium());
        metadata.put("yearCreated", artwork.getYearCreated());
        metadata.put("artistDid", artwork.getArtistDid());
        metadata.put("sha256Hash", artwork.getSha256Hash());
        metadata.put("image", imagePin.uri());
        return metadata;
    }

    /**
     * Mints an on-chain certificate for a previously-pinned artwork, enforcing KYC verification
     * and duplicate-content-hash protection before sending the transaction.
     */
    public Certificate mintCertificate(UUID artworkId, String recipientAddress) {
        Artwork artwork = artworkRepository.findById(artworkId)
                .orElseThrow(() -> new ArtworkNotFoundException(artworkId));

        if (artwork.getStatus() != ArtworkStatus.PINNED) {
            throw new ArtworkNotPinnedException(artworkId);
        }

        if (!kycVerificationService.isVerified(artwork.getArtistWalletAddress())) {
            throw new NotVerifiedException(artwork.getArtistWalletAddress());
        }

        certificateRepository.findByContentHashHex(artwork.getSha256Hash())
                .ifPresent(existing -> {
                    throw new DuplicateContentHashException(artwork.getSha256Hash());
                });

        MintResult mintResult = contractService.mintCertificate(
                recipientAddress,
                artwork.getMetadataIpfsUri(),
                artwork.getSha256Hash(),
                artwork.getArtistWalletAddress(),
                BigInteger.valueOf(artwork.getRoyaltyPercentageBps()));

        Certificate certificate = new Certificate(
                mintResult.tokenId().longValueExact(),
                artwork,
                artwork.getSha256Hash(),
                mintResult.contractAddress(),
                mintResult.txHash(),
                recipientAddress,
                artwork.getRoyaltyPercentageBps(),
                artwork.getArtistWalletAddress(),
                Instant.now());

        artwork.setStatus(ArtworkStatus.MINTED);
        artworkRepository.save(artwork);

        return certificateRepository.save(certificate);
    }

    public Certificate getCertificate(Long tokenId) {
        return certificateRepository.findById(tokenId)
                .orElseThrow(() -> new CertificateNotFoundException(tokenId));
    }

    public List<Certificate> getCertificatesForWallet(String walletAddress) {
        return certificateRepository.findByOwnerAddress(walletAddress);
    }
}
