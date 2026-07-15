package com.gandarych.nftcerts.identity;

/**
 * Artist identity verification flow (DID / KYC provider integration). Confirms that the minting
 * entity is the rightful copyright holder of the photograph before a certificate can be minted.
 */
public interface KycVerificationService {

    /** Verifies (or upserts as verified) the given wallet/DID/email tuple, returning the persisted identity. */
    ArtistIdentity verify(String walletAddress, String did, String email);

    /** Returns whether the given wallet address currently has a verified identity on file. */
    boolean isVerified(String walletAddress);
}
