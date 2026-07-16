#include "storage/PinataIpfsStorageService.h"

#include "HttpTestServer.h"
#include "error/Exceptions.h"

#include <gtest/gtest.h>

using namespace nftcerts;
using testsupport::HttpTestServer;

namespace {

class PinataStorageTest : public ::testing::Test {
protected:
    void SetUp() override { HttpTestServer::resetPinataBehavior(); }
    void TearDown() override { HttpTestServer::resetPinataBehavior(); }

    config::PinataProperties propsWithJwt() {
        config::PinataProperties props;
        props.jwt = "test-jwt";
        props.baseUrl = HttpTestServer::instance().baseUrl();
        return props;
    }
};

}  // namespace

TEST_F(PinataStorageTest, PinFileSendsBearerJwtAndParsesCid) {
    storage::PinataIpfsStorageService service(propsWithJwt());
    storage::PinResult result = service.pinFile({1, 2, 3}, "photo.jpg", "image/jpeg");

    EXPECT_EQ(result.cid, "QmFakePinataCid");
    EXPECT_EQ(result.uri, "ipfs://QmFakePinataCid");
    EXPECT_EQ(HttpTestServer::lastPinataHeaders["authorization"], "Bearer test-jwt");
}

TEST_F(PinataStorageTest, PinJsonSendsApiKeyPairHeaders) {
    config::PinataProperties props;
    props.apiKey = "key-123";
    props.apiSecret = "secret-456";
    props.baseUrl = HttpTestServer::instance().baseUrl();
    storage::PinataIpfsStorageService service(props);

    Json::Value payload;
    payload["name"] = "metadata";
    storage::PinResult result = service.pinJson(payload, "artwork-metadata");

    EXPECT_EQ(result.cid, "QmFakePinataCid");
    EXPECT_EQ(HttpTestServer::lastPinataHeaders["pinata_api_key"], "key-123");
    EXPECT_EQ(HttpTestServer::lastPinataHeaders["pinata_secret_api_key"], "secret-456");
}

TEST_F(PinataStorageTest, MissingCredentialsThrowWithoutHttpCall) {
    config::PinataProperties props;
    props.baseUrl = "http://127.0.0.1:1";  // would fail if it were ever dialed
    storage::PinataIpfsStorageService service(props);

    EXPECT_THROW(service.pinFile({1}, "f", "image/jpeg"), error::IpfsPinningException);
    EXPECT_THROW(service.pinJson(Json::Value(), "n"), error::IpfsPinningException);
}

TEST_F(PinataStorageTest, Non2xxResponseThrows) {
    HttpTestServer::pinataStatusOverride = 401;
    storage::PinataIpfsStorageService service(propsWithJwt());
    EXPECT_THROW(service.pinFile({1}, "f", "image/jpeg"), error::IpfsPinningException);
    EXPECT_THROW(service.pinJson(Json::Value(), "n"), error::IpfsPinningException);
}

TEST_F(PinataStorageTest, ResponseWithoutIpfsHashThrows) {
    HttpTestServer::pinataOmitIpfsHash = true;
    storage::PinataIpfsStorageService service(propsWithJwt());
    EXPECT_THROW(service.pinFile({1}, "f", "image/jpeg"), error::IpfsPinningException);
}

TEST_F(PinataStorageTest, UnreachableEndpointThrows) {
    config::PinataProperties props;
    props.jwt = "test-jwt";
    props.baseUrl = "http://127.0.0.1:1";
    storage::PinataIpfsStorageService service(props);
    EXPECT_THROW(service.pinFile({1}, "f", "image/jpeg"), error::IpfsPinningException);
}
