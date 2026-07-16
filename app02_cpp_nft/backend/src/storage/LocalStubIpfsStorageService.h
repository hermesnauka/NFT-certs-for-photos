#pragma once

#include "storage/IpfsStorageService.h"

#include <filesystem>

// Local, no-network stand-in for IpfsStorageService. Active when APP_STORAGE_PROVIDER=mock —
// either for automated tests or a manual dev run without Pinata credentials. Writes content to a
// local temp directory and returns a deterministic fake "ipfs://stub-<sha256>" URI. Mirrors
// app01's LocalStubIpfsStorageService.
namespace nftcerts::storage {

class LocalStubIpfsStorageService : public IpfsStorageService {
public:
    LocalStubIpfsStorageService();

    PinResult pinFile(const std::vector<unsigned char>& content, const std::string& filename,
                       const std::string& contentType) override;
    PinResult pinJson(const Json::Value& jsonPayload, const std::string& name) override;

private:
    PinResult storeAndBuildResult(const std::vector<unsigned char>& content);

    std::filesystem::path storageDir_;
};

}  // namespace nftcerts::storage
