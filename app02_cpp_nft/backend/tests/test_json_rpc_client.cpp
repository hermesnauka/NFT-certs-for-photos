#include "blockchain/JsonRpcClient.h"

#include "HttpTestServer.h"
#include "error/Exceptions.h"

#include <gtest/gtest.h>

using namespace nftcerts;
using testsupport::HttpTestServer;

namespace {

class JsonRpcClientTest : public ::testing::Test {
protected:
    void TearDown() override { HttpTestServer::resetRpcOverride(); }
};

}  // namespace

TEST_F(JsonRpcClientTest, EthChainIdParsesHexResult) {
    blockchain::JsonRpcClient client(HttpTestServer::instance().baseUrl());
    EXPECT_EQ(client.ethChainId(), 31337u);
}

TEST_F(JsonRpcClientTest, EthGetTransactionCountParsesHexResult) {
    blockchain::JsonRpcClient client(HttpTestServer::instance().baseUrl());
    EXPECT_EQ(client.ethGetTransactionCount("0x" + std::string(40, 'a')), 0u);
}

TEST_F(JsonRpcClientTest, EthCallReturnsRawHexString) {
    HttpTestServer::rpcOverride = [](const std::string& method, const Json::Value& params, Json::Value& result,
                                      std::string&) {
        if (method != "eth_call") return false;
        EXPECT_EQ(params[0]["to"].asString(), "0xto");
        EXPECT_EQ(params[1].asString(), "latest");
        result = "0x1234";
        return true;
    };
    blockchain::JsonRpcClient client(HttpTestServer::instance().baseUrl());
    EXPECT_EQ(client.ethCall("0xto", "0xdata"), "0x1234");
}

TEST_F(JsonRpcClientTest, JsonRpcErrorThrowsMintingExceptionWithMethodAndMessage) {
    HttpTestServer::rpcOverride = [](const std::string& method, const Json::Value&, Json::Value&,
                                      std::string& errorMessage) {
        if (method != "eth_sendRawTransaction") return false;
        errorMessage = "execution reverted: something broke";
        return true;
    };
    blockchain::JsonRpcClient client(HttpTestServer::instance().baseUrl());
    try {
        client.ethSendRawTransaction("0xdead");
        FAIL() << "expected MintingException";
    } catch (const error::MintingException& e) {
        std::string what = e.what();
        EXPECT_NE(what.find("eth_sendRawTransaction"), std::string::npos);
        EXPECT_NE(what.find("execution reverted: something broke"), std::string::npos);
    }
}

TEST_F(JsonRpcClientTest, UnreachableEndpointThrowsChainUnavailable) {
    blockchain::JsonRpcClient client("http://127.0.0.1:1");
    EXPECT_THROW(client.ethChainId(), error::ChainUnavailableException);
}
