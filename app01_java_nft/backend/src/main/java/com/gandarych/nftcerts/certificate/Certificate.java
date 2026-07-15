package com.gandarych.nftcerts.certificate;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.FetchType;
import jakarta.persistence.Id;
import jakarta.persistence.JoinColumn;
import jakarta.persistence.OneToOne;
import jakarta.persistence.Table;

import java.time.Instant;

@Entity
@Table(name = "certificate")
public class Certificate {

    @Id
    private Long tokenId;

    // EAGER: CertificateDtoMapper always reads artwork fields (title, IPFS URIs) to build the DTO,
    // and with open-in-view disabled there is no session left to resolve a lazy proxy by the time
    // the controller layer maps the entity returned from CertificateService.
    @OneToOne(fetch = FetchType.EAGER)
    @JoinColumn(name = "artwork_id", nullable = false, unique = true)
    private Artwork artwork;

    @Column(nullable = false, length = 64)
    private String contentHashHex;

    @Column(nullable = false)
    private String contractAddress;

    @Column(nullable = false)
    private String txHash;

    @Column(nullable = false)
    private String ownerAddress;

    @Column(nullable = false)
    private Integer royaltyPercentageBps;

    @Column(nullable = false)
    private String royaltyRecipient;

    @Column(nullable = false)
    private Instant mintedAt;

    protected Certificate() {
        // JPA
    }

    public Certificate(Long tokenId, Artwork artwork, String contentHashHex, String contractAddress,
                        String txHash, String ownerAddress, Integer royaltyPercentageBps,
                        String royaltyRecipient, Instant mintedAt) {
        this.tokenId = tokenId;
        this.artwork = artwork;
        this.contentHashHex = contentHashHex;
        this.contractAddress = contractAddress;
        this.txHash = txHash;
        this.ownerAddress = ownerAddress;
        this.royaltyPercentageBps = royaltyPercentageBps;
        this.royaltyRecipient = royaltyRecipient;
        this.mintedAt = mintedAt;
    }

    public Long getTokenId() {
        return tokenId;
    }

    public Artwork getArtwork() {
        return artwork;
    }

    public String getContentHashHex() {
        return contentHashHex;
    }

    public String getContractAddress() {
        return contractAddress;
    }

    public String getTxHash() {
        return txHash;
    }

    public String getOwnerAddress() {
        return ownerAddress;
    }

    public Integer getRoyaltyPercentageBps() {
        return royaltyPercentageBps;
    }

    public String getRoyaltyRecipient() {
        return royaltyRecipient;
    }

    public Instant getMintedAt() {
        return mintedAt;
    }
}
