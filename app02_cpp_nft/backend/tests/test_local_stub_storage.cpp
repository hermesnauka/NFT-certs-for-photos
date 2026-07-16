#include "storage/LocalStubIpfsStorageService.h"

#include "hashing/Sha256HashingService.h"

#include <gtest/gtest.h>

using namespace nftcerts;

TEST(LocalStubIpfsStorageServiceTest, PinFileReturnsDeterministicStubCid) {
    storage::LocalStubIpfsStorageService service;
    std::vector<unsigned char> content{1, 2, 3, 4};

    storage::PinResult first = service.pinFile(content, "photo.jpg", "image/jpeg");
    storage::PinResult second = service.pinFile(content, "other-name.jpg", "image/jpeg");

    hashing::Sha256HashingService hashing;
    EXPECT_EQ(first.cid, "stub-" + hashing.sha256Hex(content));
    EXPECT_EQ(first.uri, "ipfs://" + first.cid);
    EXPECT_EQ(first.cid, second.cid) << "same content must pin to the same stub CID";
}

TEST(LocalStubIpfsStorageServiceTest, DifferentContentGetsDifferentCid) {
    storage::LocalStubIpfsStorageService service;
    storage::PinResult a = service.pinFile({1, 2, 3}, "a", "image/png");
    storage::PinResult b = service.pinFile({4, 5, 6}, "b", "image/png");
    EXPECT_NE(a.cid, b.cid);
}

TEST(LocalStubIpfsStorageServiceTest, PinJsonReturnsStubCid) {
    storage::LocalStubIpfsStorageService service;
    Json::Value payload;
    payload["name"] = "Artwork";
    payload["image"] = "ipfs://QmImage";

    storage::PinResult result = service.pinJson(payload, "metadata");
    EXPECT_EQ(result.cid.rfind("stub-", 0), 0u) << "CID must start with 'stub-': " << result.cid;
    EXPECT_EQ(result.uri, "ipfs://" + result.cid);

    // Deterministic for identical payloads.
    EXPECT_EQ(service.pinJson(payload, "metadata").cid, result.cid);
}
