package com.devpowers.nftcerts.identity;

import org.springframework.stereotype.Service;

import java.time.Instant;
import java.util.UUID;

/**
 * Mock KYC provider integration: always approves verification. Stands in for a real DID/KYC
 * provider (e.g. Persona, Civic) that would be wired in a production deployment.
 */
@Service
public class MockKycVerificationService implements KycVerificationService {

    private final ArtistIdentityRepository artistIdentityRepository;

    public MockKycVerificationService(ArtistIdentityRepository artistIdentityRepository) {
        this.artistIdentityRepository = artistIdentityRepository;
    }

    @Override
    public ArtistIdentity verify(String walletAddress, String did, String email) {
        ArtistIdentity identity = artistIdentityRepository.findByWalletAddress(walletAddress)
                .orElseGet(() -> new ArtistIdentity(UUID.randomUUID(), walletAddress, did, email, false, null));

        identity.setDid(did);
        identity.setEmail(email);
        identity.setVerified(true);
        identity.setVerifiedAt(Instant.now());

        return artistIdentityRepository.save(identity);
    }

    @Override
    public boolean isVerified(String walletAddress) {
        return artistIdentityRepository.findByWalletAddress(walletAddress)
                .map(ArtistIdentity::isVerified)
                .orElse(false);
    }
}
