#pragma once

#include "identity/ArtistIdentityRepository.h"
#include "identity/KycVerificationService.h"

// Mock KYC provider integration: always approves verification. Stands in for a real DID/KYC
// provider (e.g. Persona, Civic) that would be wired in a production deployment.
namespace nftcerts::identity {

class MockKycVerificationService : public KycVerificationService {
public:
    explicit MockKycVerificationService(ArtistIdentityRepository& repository) : repository_(repository) {}

    ArtistIdentity verify(const std::string& walletAddress, const std::string& did,
                           const std::string& email) override;
    bool isVerified(const std::string& walletAddress) override;

private:
    ArtistIdentityRepository& repository_;
};

}  // namespace nftcerts::identity
