#include "HttpTestServer.h"

#include "hashing/Sha256HashingService.h"

#include <gtest/gtest.h>

#include <string>

using namespace nftcerts;
using testsupport::HttpTestServer;

namespace {

Json::Value parseBody(const drogon::HttpResponsePtr& response) {
    auto json = response->getJsonObject();
    EXPECT_NE(json, nullptr) << "response is not JSON: " << response->getBody();
    return json ? *json : Json::Value();
}

// Asserts the app01-compatible error JSON contract produced by ExceptionMapping/ErrorResponse.
void expectErrorShape(const drogon::HttpResponsePtr& response, int status, const std::string& path) {
    EXPECT_EQ(response->getStatusCode(), status);
    Json::Value body = parseBody(response);
    EXPECT_EQ(body["status"].asInt(), status);
    EXPECT_EQ(body["path"].asString(), path);
    EXPECT_FALSE(body["error"].asString().empty());
    EXPECT_FALSE(body["message"].asString().empty());
    EXPECT_FALSE(body["timestamp"].asString().empty());
}

std::string verifyWallet(HttpTestServer& server, const std::string& wallet) {
    Json::Value body;
    body["walletAddress"] = wallet;
    body["did"] = "did:key:z6MkHttpTest";
    body["email"] = "artist@example.com";
    auto response = server.postJson("/api/identity/verify", body);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);
    return wallet;
}

}  // namespace

TEST(ApiHttpTest, FullMintFlowEndToEnd) {
    HttpTestServer& server = HttpTestServer::instance();
    std::string wallet = verifyWallet(server, "0x" + std::string(40, 'a'));

    // 1. Upload (PNG so the EXIF watermarker passes the bytes through untouched).
    std::string fileContent = "fake-png-bytes-e2e";
    auto uploadResponse = server.postMultipartFile("/api/uploads", "photo.png", fileContent);
    ASSERT_EQ(uploadResponse->getStatusCode(), drogon::k200OK) << uploadResponse->getBody();
    Json::Value upload = parseBody(uploadResponse);
    EXPECT_EQ(upload["originalFilename"].asString(), "photo.png");
    hashing::Sha256HashingService hashing;
    std::string expectedHash =
        hashing.sha256Hex(std::vector<unsigned char>(fileContent.begin(), fileContent.end()));
    EXPECT_EQ(upload["sha256Hash"].asString(), expectedHash);
    std::string fileId = upload["fileId"].asString();
    ASSERT_FALSE(fileId.empty());

    // 2. Create artwork — pins to the local stub and persists as PINNED.
    Json::Value artworkRequest;
    artworkRequest["fileId"] = fileId;
    artworkRequest["title"] = "E2E Artwork";
    artworkRequest["description"] = "desc";
    artworkRequest["medium"] = "Photography";
    artworkRequest["yearCreated"] = 2024;
    artworkRequest["royaltyPercentageBps"] = 500;
    artworkRequest["artistWalletAddress"] = wallet;
    artworkRequest["artistDid"] = "did:key:z6MkHttpTest";
    auto artworkResponse = server.postJson("/api/artworks", artworkRequest);
    ASSERT_EQ(artworkResponse->getStatusCode(), drogon::k201Created) << artworkResponse->getBody();
    Json::Value artwork = parseBody(artworkResponse);
    EXPECT_EQ(artwork["status"].asString(), "PINNED");
    EXPECT_EQ(artwork["sha256Hash"].asString(), expectedHash);
    EXPECT_EQ(artwork["imageIpfsUri"].asString().rfind("ipfs://stub-", 0), 0u);
    EXPECT_EQ(artwork["metadataIpfsUri"].asString().rfind("ipfs://stub-", 0), 0u);
    std::string artworkId = artwork["artworkId"].asString();

    // 3. Mint — goes through the real ContractService against the fake chain.
    Json::Value mintRequest;
    mintRequest["recipientAddress"] = wallet;
    auto mintResponse = server.postJson("/api/artworks/" + artworkId + "/mint", mintRequest);
    ASSERT_EQ(mintResponse->getStatusCode(), drogon::k200OK) << mintResponse->getBody();
    Json::Value cert = parseBody(mintResponse);
    uint64_t tokenId = cert["tokenId"].asUInt64();
    EXPECT_GT(tokenId, 0u);
    EXPECT_EQ(cert["artworkId"].asString(), artworkId);
    EXPECT_EQ(cert["contractAddress"].asString(), HttpTestServer::contractAddress());
    EXPECT_EQ(cert["txHash"].asString().rfind("0xfaketxhash", 0), 0u);
    EXPECT_EQ(cert["contentHashHex"].asString(), expectedHash);
    EXPECT_EQ(cert["etherscanUrl"].asString().rfind("http://etherscan.test/tx/", 0), 0u);
    EXPECT_EQ(cert["openSeaUrl"].asString().rfind("http://opensea.test/assets/", 0), 0u);
    EXPECT_EQ(cert["raribleUrl"].asString().rfind("http://rarible.test/token/", 0), 0u);

    // 4. Fetch the certificate by tokenId.
    auto getResponse = server.get("/api/certificates/" + std::to_string(tokenId));
    ASSERT_EQ(getResponse->getStatusCode(), drogon::k200OK);
    Json::Value fetched = parseBody(getResponse);
    EXPECT_EQ(fetched["tokenId"].asUInt64(), tokenId);
    EXPECT_EQ(fetched["title"].asString(), "E2E Artwork");

    // 5. Download the PDF certificate.
    auto pdfResponse = server.get("/api/certificates/" + std::to_string(tokenId) + "/pdf");
    ASSERT_EQ(pdfResponse->getStatusCode(), drogon::k200OK);
    EXPECT_EQ(pdfResponse->getHeader("Content-Type"), "application/pdf");
    EXPECT_NE(pdfResponse->getHeader("Content-Disposition").find("certificate-"), std::string::npos);
    std::string pdfBody(pdfResponse->getBody());
    ASSERT_GT(pdfBody.size(), 500u);
    EXPECT_EQ(pdfBody.substr(0, 5), "%PDF-");

    // 6. Dashboard lists the certificate for the recipient wallet.
    auto dashboardResponse = server.get("/api/artists/" + wallet + "/dashboard");
    ASSERT_EQ(dashboardResponse->getStatusCode(), drogon::k200OK);
    Json::Value dashboard = parseBody(dashboardResponse);
    EXPECT_EQ(dashboard["walletAddress"].asString(), wallet);
    EXPECT_GE(dashboard["totalCertificates"].asInt(), 1);
    ASSERT_TRUE(dashboard["certificates"].isArray());
    EXPECT_GE(dashboard["certificates"].size(), 1u);

    // 7. Re-minting the same artwork is rejected: its status is MINTED (no longer PINNED), which
    // the service checks before the duplicate-content-hash rule, so this maps to 422. The 409
    // duplicate path is covered by CertificateServiceTest and ContractServiceTest directly.
    auto duplicateMint = server.postJson("/api/artworks/" + artworkId + "/mint", mintRequest);
    expectErrorShape(duplicateMint, 422, "/api/artworks/" + artworkId + "/mint");
}

TEST(ApiHttpTest, UploadWithoutFileReturns400) {
    HttpTestServer& server = HttpTestServer::instance();
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Post);
    request->setPath("/api/uploads");
    auto response = server.sendSync(request);
    expectErrorShape(response, 400, "/api/uploads");
}

TEST(ApiHttpTest, UploadUnsupportedExtensionReturns400) {
    HttpTestServer& server = HttpTestServer::instance();
    auto response = server.postMultipartFile("/api/uploads", "animation.gif", "gif-bytes");
    expectErrorShape(response, 400, "/api/uploads");
    EXPECT_NE(parseBody(response)["message"].asString().find("Unsupported file type"), std::string::npos);
}

TEST(ApiHttpTest, CreateArtworkWithUnknownFileIdReturns404) {
    HttpTestServer& server = HttpTestServer::instance();
    Json::Value body;
    body["fileId"] = "does-not-exist";
    body["title"] = "T";
    body["royaltyPercentageBps"] = 500;
    body["artistWalletAddress"] = "0x" + std::string(40, 'b');
    auto response = server.postJson("/api/artworks", body);
    expectErrorShape(response, 404, "/api/artworks");
}

TEST(ApiHttpTest, CreateArtworkWithMissingTitleReturns422) {
    HttpTestServer& server = HttpTestServer::instance();
    Json::Value body;
    body["fileId"] = "irrelevant";
    body["royaltyPercentageBps"] = 500;
    body["artistWalletAddress"] = "0x" + std::string(40, 'b');
    auto response = server.postJson("/api/artworks", body);
    expectErrorShape(response, 422, "/api/artworks");
    EXPECT_NE(parseBody(response)["message"].asString().find("title"), std::string::npos);
}

TEST(ApiHttpTest, MintUnknownArtworkReturns404) {
    HttpTestServer& server = HttpTestServer::instance();
    Json::Value body;
    body["recipientAddress"] = "0x" + std::string(40, 'c');
    auto response = server.postJson("/api/artworks/no-such-artwork/mint", body);
    expectErrorShape(response, 404, "/api/artworks/no-such-artwork/mint");
}

TEST(ApiHttpTest, UnknownCertificateReturns404) {
    HttpTestServer& server = HttpTestServer::instance();
    expectErrorShape(server.get("/api/certificates/999999"), 404, "/api/certificates/999999");
    // Non-numeric token ids are mapped to 404 as well, not 500.
    expectErrorShape(server.get("/api/certificates/abc"), 404, "/api/certificates/abc");
}

TEST(ApiHttpTest, I18nServesEnAndPlAndRejectsUnknownLanguage) {
    HttpTestServer& server = HttpTestServer::instance();

    auto en = server.get("/api/i18n/en");
    ASSERT_EQ(en->getStatusCode(), drogon::k200OK);
    EXPECT_TRUE(parseBody(en).isMember("certificate.authenticity.body"));

    auto pl = server.get("/api/i18n/pl");
    ASSERT_EQ(pl->getStatusCode(), drogon::k200OK);
    EXPECT_TRUE(parseBody(pl).isMember("certificate.authenticity.body"));

    expectErrorShape(server.get("/api/i18n/de"), 404, "/api/i18n/de");
}

TEST(ApiHttpTest, IdentityVerifyHappyPathAndValidation) {
    HttpTestServer& server = HttpTestServer::instance();

    Json::Value valid;
    valid["walletAddress"] = "0x" + std::string(40, 'd');
    valid["did"] = "did:key:z6MkValid";
    auto ok = server.postJson("/api/identity/verify", valid);
    ASSERT_EQ(ok->getStatusCode(), drogon::k200OK);
    Json::Value body = parseBody(ok);
    EXPECT_TRUE(body["verified"].asBool());
    EXPECT_EQ(body["walletAddress"].asString(), valid["walletAddress"].asString());
    EXPECT_EQ(body["did"].asString(), "did:key:z6MkValid");

    Json::Value badWallet = valid;
    badWallet["walletAddress"] = "not-a-wallet";
    expectErrorShape(server.postJson("/api/identity/verify", badWallet), 400, "/api/identity/verify");

    Json::Value badDid = valid;
    badDid["did"] = "not-a-did";
    expectErrorShape(server.postJson("/api/identity/verify", badDid), 400, "/api/identity/verify");

    Json::Value missingDid;
    missingDid["walletAddress"] = valid["walletAddress"];
    expectErrorShape(server.postJson("/api/identity/verify", missingDid), 400, "/api/identity/verify");
}

TEST(ApiHttpTest, DashboardForUnknownWalletIsEmpty) {
    HttpTestServer& server = HttpTestServer::instance();
    auto response = server.get("/api/artists/0xnobody/dashboard");
    ASSERT_EQ(response->getStatusCode(), drogon::k200OK);
    Json::Value body = parseBody(response);
    EXPECT_EQ(body["totalCertificates"].asInt(), 0);
    EXPECT_EQ(body["certificates"].size(), 0u);
}
