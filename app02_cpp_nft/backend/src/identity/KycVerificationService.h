#pragma once

#include "identity/ArtistIdentity.h"

#include <string>

// Artist identity verification flow (DID / KYC provider integration). Confirms that the minting
// entity is the rightful copyright holder of the photograph before a certificate can be minted.
namespace nftcerts::identity {

class KycVerificationService {
public:
    virtual ~KycVerificationService() = default;

    // Verifies (or upserts as verified) the given wallet/DID/email tuple, returning the persisted identity.
    virtual ArtistIdentity verify(const std::string& walletAddress, const std::string& did,
                                   const std::string& email) = 0;

    // Returns whether the given wallet address currently has a verified identity on file.
    virtual bool isVerified(const std::string& walletAddress) = 0;
};

}  // namespace nftcerts::identity
