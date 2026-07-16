#include "api/UploadStore.h"

#include "error/Exceptions.h"

#include <gtest/gtest.h>

using namespace nftcerts;

TEST(UploadStoreTest, StoreThenGetRoundTrip) {
    api::UploadStore store;
    certificate::StoredUpload upload{"photo.jpg", "image/jpeg", std::string(64, 'a'), {1, 2, 3, 4}};

    std::string fileId = store.store(upload);
    EXPECT_FALSE(fileId.empty());

    certificate::StoredUpload retrieved = store.get(fileId);
    EXPECT_EQ(retrieved.originalFilename, upload.originalFilename);
    EXPECT_EQ(retrieved.contentType, upload.contentType);
    EXPECT_EQ(retrieved.sha256Hash, upload.sha256Hash);
    EXPECT_EQ(retrieved.content, upload.content);
}

TEST(UploadStoreTest, DistinctIdsForDistinctStores) {
    api::UploadStore store;
    certificate::StoredUpload upload{"a.png", "image/png", std::string(64, 'b'), {9}};
    std::string first = store.store(upload);
    std::string second = store.store(upload);
    EXPECT_NE(first, second);
}

TEST(UploadStoreTest, GetUnknownFileIdThrows) {
    api::UploadStore store;
    EXPECT_THROW(store.get("no-such-file-id"), error::FileNotFoundInUploadStoreException);
}
