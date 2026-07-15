package com.gandarych.nftcerts.certificate;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;

public interface CertificateRepository extends JpaRepository<Certificate, Long> {

    Optional<Certificate> findByContentHashHex(String contentHashHex);

    List<Certificate> findByOwnerAddress(String ownerAddress);
}
