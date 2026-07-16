#include "storage/LocalStubIpfsStorageService.h"

#include "hashing/Sha256HashingService.h"

#include <json/json.h>

#include <fstream>

namespace nftcerts::storage {

LocalStubIpfsStorageService::LocalStubIpfsStorageService() {
    storageDir_ = std::filesystem::temp_directory_path() / "local-stub-ipfs";
    std::filesystem::create_directories(storageDir_);
}

PinResult LocalStubIpfsStorageService::pinFile(const std::vector<unsigned char>& content, const std::string&,
                                                const std::string&) {
    return storeAndBuildResult(content);
}

PinResult LocalStubIpfsStorageService::pinJson(const Json::Value& jsonPayload, const std::string&) {
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    std::string serialized = Json::writeString(writer, jsonPayload);
    std::vector<unsigned char> content(serialized.begin(), serialized.end());
    return storeAndBuildResult(content);
}

PinResult LocalStubIpfsStorageService::storeAndBuildResult(const std::vector<unsigned char>& content) {
    hashing::Sha256HashingService hashingService;
    std::string cid = "stub-" + hashingService.sha256Hex(content);

    std::ofstream file(storageDir_ / cid, std::ios::binary);
    file.write(reinterpret_cast<const char*>(content.data()), static_cast<std::streamsize>(content.size()));

    return PinResult::of(cid);
}

}  // namespace nftcerts::storage
