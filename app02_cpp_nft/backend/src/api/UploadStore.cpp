#include "api/UploadStore.h"

#include "error/Exceptions.h"
#include "util/Uuid.h"

namespace nftcerts::api {

std::string UploadStore::store(const certificate::StoredUpload& upload) {
    std::string fileId = util::randomUuid();
    std::lock_guard<std::mutex> lock(mutex_);
    uploads_[fileId] = upload;
    return fileId;
}

certificate::StoredUpload UploadStore::get(const std::string& fileId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = uploads_.find(fileId);
    if (it == uploads_.end()) {
        throw error::FileNotFoundInUploadStoreException(fileId);
    }
    return it->second;
}

}  // namespace nftcerts::api
