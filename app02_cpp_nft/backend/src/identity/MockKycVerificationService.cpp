#include "identity/MockKycVerificationService.h"

#include "util/Time.h"
#include "util/Uuid.h"

namespace nftcerts::identity {

ArtistIdentity MockKycVerificationService::verify(const std::string& walletAddress, const std::string& did,
                                                    const std::string& email) {
    ArtistIdentity identity;
    auto existing = repository_.findByWalletAddress(walletAddress);
    if (existing.has_value()) {
        identity = *existing;
    } else {
        identity.id = util::randomUuid();
        identity.walletAddress = walletAddress;
    }

    identity.did = did;
    identity.email = email;
    identity.verified = true;
    identity.verifiedAt = util::nowIso8601();

    return repository_.save(identity);
}

bool MockKycVerificationService::isVerified(const std::string& walletAddress) {
    auto identity = repository_.findByWalletAddress(walletAddress);
    return identity.has_value() && identity->verified;
}

}  // namespace nftcerts::identity
