#pragma once

#include "storage/PinResult.h"

#include <json/json.h>

#include <string>
#include <vector>

// Abstraction over decentralized (IPFS) storage pinning. Two implementations exist:
// PinataIpfsStorageService (real, used outside the mock storage profile) and
// LocalStubIpfsStorageService (local stub, wired only when APP_STORAGE_PROVIDER=mock).
namespace nftcerts::storage {

class IpfsStorageService {
public:
    virtual ~IpfsStorageService() = default;

    // Pins raw file bytes (e.g. the source image) to IPFS.
    virtual PinResult pinFile(const std::vector<unsigned char>& content, const std::string& filename,
                               const std::string& contentType) = 0;

    // Pins a JSON-serializable payload (e.g. NFT metadata) to IPFS.
    virtual PinResult pinJson(const Json::Value& jsonPayload, const std::string& name) = 0;
};

}  // namespace nftcerts::storage
