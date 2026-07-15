package com.gandarych.nftcerts.certificate;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;
import java.util.UUID;

public interface ArtworkRepository extends JpaRepository<Artwork, UUID> {

    Optional<Artwork> findBySha256Hash(String sha256Hash);
}
