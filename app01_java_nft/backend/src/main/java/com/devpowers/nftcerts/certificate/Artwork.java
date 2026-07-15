package com.devpowers.nftcerts.certificate;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.EnumType;
import jakarta.persistence.Enumerated;
import jakarta.persistence.Id;
import jakarta.persistence.Index;
import jakarta.persistence.Table;

import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "artwork", indexes = @Index(name = "ux_artwork_sha256", columnList = "sha256Hash", unique = true))
public class Artwork {

    @Id
    private UUID id;

    @Column(nullable = false)
    private UUID fileId;

    @Column(nullable = false)
    private String originalFilename;

    @Column(nullable = false, unique = true, length = 64)
    private String sha256Hash;

    private String title;

    @Column(length = 4000)
    private String description;

    private String medium;

    private Integer yearCreated;

    @Column(nullable = false)
    private Integer royaltyPercentageBps;

    @Column(nullable = false)
    private String artistWalletAddress;

    private String artistDid;

    private String imageIpfsUri;

    private String metadataIpfsUri;

    @Enumerated(EnumType.STRING)
    @Column(nullable = false)
    private ArtworkStatus status;

    @Column(nullable = false)
    private Instant createdAt;

    protected Artwork() {
        // JPA
    }

    public Artwork(UUID id, UUID fileId, String originalFilename, String sha256Hash, String title,
                   String description, String medium, Integer yearCreated, Integer royaltyPercentageBps,
                   String artistWalletAddress, String artistDid, ArtworkStatus status, Instant createdAt) {
        this.id = id;
        this.fileId = fileId;
        this.originalFilename = originalFilename;
        this.sha256Hash = sha256Hash;
        this.title = title;
        this.description = description;
        this.medium = medium;
        this.yearCreated = yearCreated;
        this.royaltyPercentageBps = royaltyPercentageBps;
        this.artistWalletAddress = artistWalletAddress;
        this.artistDid = artistDid;
        this.status = status;
        this.createdAt = createdAt;
    }

    public UUID getId() {
        return id;
    }

    public UUID getFileId() {
        return fileId;
    }

    public String getOriginalFilename() {
        return originalFilename;
    }

    public String getSha256Hash() {
        return sha256Hash;
    }

    public String getTitle() {
        return title;
    }

    public String getDescription() {
        return description;
    }

    public String getMedium() {
        return medium;
    }

    public Integer getYearCreated() {
        return yearCreated;
    }

    public Integer getRoyaltyPercentageBps() {
        return royaltyPercentageBps;
    }

    public String getArtistWalletAddress() {
        return artistWalletAddress;
    }

    public String getArtistDid() {
        return artistDid;
    }

    public String getImageIpfsUri() {
        return imageIpfsUri;
    }

    public void setImageIpfsUri(String imageIpfsUri) {
        this.imageIpfsUri = imageIpfsUri;
    }

    public String getMetadataIpfsUri() {
        return metadataIpfsUri;
    }

    public void setMetadataIpfsUri(String metadataIpfsUri) {
        this.metadataIpfsUri = metadataIpfsUri;
    }

    public ArtworkStatus getStatus() {
        return status;
    }

    public void setStatus(ArtworkStatus status) {
        this.status = status;
    }

    public Instant getCreatedAt() {
        return createdAt;
    }
}
