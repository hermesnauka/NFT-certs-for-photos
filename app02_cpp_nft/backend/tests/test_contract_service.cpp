#include "blockchain/ContractService.h"

#include "HttpTestServer.h"
#include "error/Exceptions.h"

#include <gtest/gtest.h>

using namespace nftcerts;
using testsupport::HttpTestServer;

namespace {

constexpr const char* kMinterKey = "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80";
constexpr const char* kRecipient = "0x70997970c51812dc3a010c7d01b50e0d17dc79c8";

class ContractServiceTest : public ::testing::Test {
protected:
    void TearDown() override { HttpTestServer::resetRpcOverride(); }

    blockchain::ContractService makeService() {
        config::Web3Properties web3;
        web3.rpcUrl = HttpTestServer::instance().baseUrl();
        web3.minterPrivateKey = kMinterKey;
        web3.contractAddress = HttpTestServer::contractAddress();
        return blockchain::ContractService(web3);
    }
};

}  // namespace

TEST_F(ContractServiceTest, HappyPathMintReturnsTokenIdAndTxHash) {
    blockchain::ContractService service = makeService();
    blockchain::MintResult result =
        service.mintCertificate(kRecipient, "ipfs://QmMeta", std::string(64, 'a'), kRecipient, 500);

    EXPECT_GT(result.tokenId, 0u);
    EXPECT_EQ(result.txHash.rfind("0xfaketxhash", 0), 0u);
    EXPECT_EQ(result.contractAddress, HttpTestServer::contractAddress());
}

TEST_F(ContractServiceTest, DuplicateContentHashRevertMapsToDuplicateException) {
    HttpTestServer::rpcOverride = [](const std::string& method, const Json::Value&, Json::Value&,
                                      std::string& errorMessage) {
        if (method != "eth_sendRawTransaction") return false;
        errorMessage = "execution reverted: PhotoCertificate: duplicate content hash";
        return true;
    };
    blockchain::ContractService service = makeService();
    EXPECT_THROW(service.mintCertificate(kRecipient, "ipfs://QmMeta", std::string(64, 'b'), kRecipient, 500),
                 error::DuplicateContentHashException);
}

TEST_F(ContractServiceTest, OtherRpcErrorMapsToMintingException) {
    HttpTestServer::rpcOverride = [](const std::string& method, const Json::Value&, Json::Value&,
                                      std::string& errorMessage) {
        if (method != "eth_sendRawTransaction") return false;
        errorMessage = "insufficient funds for gas";
        return true;
    };
    blockchain::ContractService service = makeService();
    EXPECT_THROW(service.mintCertificate(kRecipient, "ipfs://QmMeta", std::string(64, 'c'), kRecipient, 500),
                 error::MintingException);
}

TEST_F(ContractServiceTest, RevertedReceiptStatusThrowsMintingException) {
    HttpTestServer::rpcOverride = [](const std::string& method, const Json::Value&, Json::Value& result,
                                      std::string&) {
        if (method != "eth_getTransactionReceipt") return false;
        result = Json::Value(Json::objectValue);
        result["status"] = "0x0";
        result["logs"] = Json::Value(Json::arrayValue);
        return true;
    };
    blockchain::ContractService service = makeService();
    EXPECT_THROW(service.mintCertificate(kRecipient, "ipfs://QmMeta", std::string(64, 'd'), kRecipient, 500),
                 error::MintingException);
}

TEST_F(ContractServiceTest, ReceiptWithoutMintedEventThrowsMintingException) {
    HttpTestServer::rpcOverride = [](const std::string& method, const Json::Value&, Json::Value& result,
                                      std::string&) {
        if (method != "eth_getTransactionReceipt") return false;
        result = Json::Value(Json::objectValue);
        result["status"] = "0x1";
        result["logs"] = Json::Value(Json::arrayValue);  // success but no CertificateMinted log
        return true;
    };
    blockchain::ContractService service = makeService();
    EXPECT_THROW(service.mintCertificate(kRecipient, "ipfs://QmMeta", std::string(64, 'e'), kRecipient, 500),
                 error::MintingException);
}

TEST_F(ContractServiceTest, MisconfiguredMinterKeyThrowsBeforeAnyRpc) {
    config::Web3Properties web3;
    web3.rpcUrl = "http://127.0.0.1:1";  // would throw ChainUnavailable if it were dialed
    web3.minterPrivateKey = "not-a-key";
    web3.contractAddress = HttpTestServer::contractAddress();
    blockchain::ContractService service(web3);
    EXPECT_THROW(service.mintCertificate(kRecipient, "ipfs://QmMeta", std::string(64, 'f'), kRecipient, 500),
                 error::MintingException);
}

TEST_F(ContractServiceTest, TokenContentHashReturnsValueFromEthCall) {
    std::string storedHash = "0x" + std::string(64, '9');
    HttpTestServer::rpcOverride = [storedHash](const std::string& method, const Json::Value&, Json::Value& result,
                                                std::string&) {
        if (method != "eth_call") return false;
        result = storedHash;
        return true;
    };
    blockchain::ContractService service = makeService();
    auto hash = service.tokenContentHash(1);
    ASSERT_TRUE(hash.has_value());
    EXPECT_EQ(*hash, storedHash);
}

TEST_F(ContractServiceTest, TokenContentHashReturnsNulloptForEmptyResult) {
    blockchain::ContractService service = makeService();  // default fake eth_call returns "0x"
    EXPECT_FALSE(service.tokenContentHash(42).has_value());
}
