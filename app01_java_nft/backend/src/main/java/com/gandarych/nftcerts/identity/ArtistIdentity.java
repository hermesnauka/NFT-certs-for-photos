package com.gandarych.nftcerts.identity;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "artist_identity", indexes = @jakarta.persistence.Index(name = "ux_artist_wallet", columnList = "walletAddress", unique = true))
public class ArtistIdentity {

    @Id
    private UUID id;

    @Column(nullable = false, unique = true)
    private String walletAddress;

    private String did;

    private String email;

    @Column(nullable = false)
    private boolean verified;

    private Instant verifiedAt;

    protected ArtistIdentity() {
        // JPA
    }

    public ArtistIdentity(UUID id, String walletAddress, String did, String email, boolean verified, Instant verifiedAt) {
        this.id = id;
        this.walletAddress = walletAddress;
        this.did = did;
        this.email = email;
        this.verified = verified;
        this.verifiedAt = verifiedAt;
    }

    public UUID getId() {
        return id;
    }

    public String getWalletAddress() {
        return walletAddress;
    }

    public void setWalletAddress(String walletAddress) {
        this.walletAddress = walletAddress;
    }

    public String getDid() {
        return did;
    }

    public void setDid(String did) {
        this.did = did;
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    public boolean isVerified() {
        return verified;
    }

    public void setVerified(boolean verified) {
        this.verified = verified;
    }

    public Instant getVerifiedAt() {
        return verifiedAt;
    }

    public void setVerifiedAt(Instant verifiedAt) {
        this.verifiedAt = verifiedAt;
    }
}
