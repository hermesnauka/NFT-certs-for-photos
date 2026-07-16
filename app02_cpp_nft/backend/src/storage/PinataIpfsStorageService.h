#pragma once

#include "config/Config.h"
#include "storage/IpfsStorageService.h"

// Real IpfsStorageService implementation backed by the Pinata pinning API. Active when
// APP_STORAGE_PROVIDER=pinata (the default). Mirrors app01's PinataIpfsStorageService.
namespace nftcerts::storage {

class PinataIpfsStorageService : public IpfsStorageService {
public:
    explicit PinataIpfsStorageService(config::PinataProperties pinataProperties);

    PinResult pinFile(const std::vector<unsigned char>& content, const std::string& filename,
                       const std::string& contentType) override;
    PinResult pinJson(const Json::Value& jsonPayload, const std::string& name) override;

private:
    config::PinataProperties pinataProperties_;

    std::string extractCid(const std::string& responseBody);
};

}  // namespace nftcerts::storage
