#pragma once

#include "certificate/CertificateService.h"

#include <mutex>
#include <string>
#include <unordered_map>

// Simple in-memory store for uploaded (hashed/watermarked) file bytes, retrievable later by
// fileId when creating an artwork. Mirrors app01's UploadStore.
namespace nftcerts::api {

class UploadStore {
public:
    std::string store(const certificate::StoredUpload& upload);
    certificate::StoredUpload get(const std::string& fileId);

private:
    std::mutex mutex_;
    std::unordered_map<std::string, certificate::StoredUpload> uploads_;
};

}  // namespace nftcerts::api
