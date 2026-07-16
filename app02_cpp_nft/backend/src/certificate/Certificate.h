#pragma once

#include "certificate/Artwork.h"

#include <cstdint>
#include <string>

// Mirrors app01's Certificate JPA entity. Represents a successfully minted on-chain NFT; the
// `artwork` member is populated eagerly by CertificateRepository (mirrors the Java entity's
// `@OneToOne(fetch = FetchType.EAGER)`), since CertificateDtoMapper always reads artwork fields.
namespace nftcerts::certificate {

struct Certificate {
    uint64_t tokenId = 0;
    Artwork artwork;
    std::string contentHashHex;
    std::string contractAddress;
    std::string txHash;
    std::string ownerAddress;
    int royaltyPercentageBps = 0;
    std::string royaltyRecipient;
    std::string mintedAt;  // ISO-8601 instant
};

}  // namespace nftcerts::certificate
