#pragma once

#include <string>

namespace nftcerts::identity {

struct ArtistIdentity {
    std::string id;  // UUID
    std::string walletAddress;
    std::string did;
    std::string email;
    bool verified = false;
    std::string verifiedAt;  // ISO-8601 instant, empty if never verified
};

}  // namespace nftcerts::identity
