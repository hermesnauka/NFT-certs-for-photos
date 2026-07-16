#pragma once

#include "certificate/ArtworkStatus.h"

#include <optional>
#include <string>

// Mirrors app01's Artwork JPA entity. Represents an uploaded photograph after hashing and
// watermarking, before (or after) it has been pinned to IPFS and minted.
namespace nftcerts::certificate {

struct Artwork {
    std::string id;        // UUID
    std::string fileId;    // UUID
    std::string originalFilename;
    std::string sha256Hash;  // hex, 64 chars, unique
    std::string title;
    std::string description;
    std::string medium;
    std::optional<int> yearCreated;
    int royaltyPercentageBps = 0;
    std::string artistWalletAddress;
    std::string artistDid;
    std::string imageIpfsUri;
    std::string metadataIpfsUri;
    ArtworkStatus status = ArtworkStatus::UPLOADED;
    std::string createdAt;  // ISO-8601 instant
};

}  // namespace nftcerts::certificate
