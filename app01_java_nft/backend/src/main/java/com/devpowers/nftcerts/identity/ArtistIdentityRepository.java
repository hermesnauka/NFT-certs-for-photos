package com.devpowers.nftcerts.identity;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;
import java.util.UUID;

public interface ArtistIdentityRepository extends JpaRepository<ArtistIdentity, UUID> {

    Optional<ArtistIdentity> findByWalletAddress(String walletAddress);
}
