#include "config/Config.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace nftcerts::config;

namespace {

// Saves and restores every environment variable loadFromEnv reads, so tests can freely
// setenv/unsetenv without leaking into other tests or depending on the invoking shell.
class ConfigEnvTest : public ::testing::Test {
protected:
    void SetUp() override {
        for (const char* name : kVars) {
            const char* value = std::getenv(name);
            saved_[name] = value ? std::optional<std::string>(value) : std::nullopt;
            unsetenv(name);
        }
    }

    void TearDown() override {
        for (const auto& [name, value] : saved_) {
            if (value.has_value()) {
                setenv(name.c_str(), value->c_str(), 1);
            } else {
                unsetenv(name.c_str());
            }
        }
    }

    static constexpr const char* kVars[] = {
        "SERVER_PORT",   "DB_PATH",           "APP_STORAGE_PROVIDER", "PINATA_JWT",
        "PINATA_API_KEY", "PINATA_API_SECRET", "PINATA_BASE_URL",      "WEB3J_RPC_URL",
        "MINTER_PRIVATE_KEY", "NFT_CONTRACT_ADDRESS", "ETHERSCAN_BASE_URL", "OPENSEA_BASE_URL",
        "RARIBLE_BASE_URL"};

private:
    std::map<std::string, std::optional<std::string>> saved_;
};

AppConfig validMockConfig() {
    AppConfig config;
    config.storage.provider = "mock";
    config.web3.contractAddress = "0x5FbDB2315678afecb367f032d93F642f64180aa3";
    config.web3.minterPrivateKey = "0x" + std::string(64, '1');
    return config;
}

}  // namespace

TEST_F(ConfigEnvTest, LoadFromEnvReadsVariables) {
    setenv("SERVER_PORT", "19999", 1);
    setenv("DB_PATH", "/tmp/test.db", 1);
    setenv("APP_STORAGE_PROVIDER", "mock", 1);
    setenv("PINATA_JWT", "test-jwt", 1);
    setenv("WEB3J_RPC_URL", "http://localhost:9999", 1);
    setenv("MINTER_PRIVATE_KEY", "0xkey", 1);
    setenv("NFT_CONTRACT_ADDRESS", "0xcontract", 1);
    setenv("ETHERSCAN_BASE_URL", "http://etherscan.test/tx/", 1);

    AppConfig config = AppConfig::loadFromEnv();

    EXPECT_EQ(config.serverPort, 19999);
    EXPECT_EQ(config.dbPath, "/tmp/test.db");
    EXPECT_EQ(config.storage.provider, "mock");
    EXPECT_TRUE(config.storage.isMock());
    EXPECT_EQ(config.pinata.jwt, "test-jwt");
    EXPECT_TRUE(config.pinata.hasJwt());
    EXPECT_EQ(config.web3.rpcUrl, "http://localhost:9999");
    EXPECT_EQ(config.web3.minterPrivateKey, "0xkey");
    EXPECT_EQ(config.web3.contractAddress, "0xcontract");
    EXPECT_EQ(config.explorer.etherscanBaseUrl, "http://etherscan.test/tx/");
}

TEST_F(ConfigEnvTest, LoadFromEnvUsesDefaultsWhenUnset) {
    AppConfig config = AppConfig::loadFromEnv();

    EXPECT_EQ(config.serverPort, 8081);
    EXPECT_EQ(config.dbPath, "data/nft-certs.db");
    EXPECT_EQ(config.storage.provider, "pinata");
    EXPECT_EQ(config.web3.rpcUrl, "http://localhost:8545");
    EXPECT_EQ(config.pinata.baseUrl, "https://api.pinata.cloud");
}

TEST(ConfigValidationTest, AcceptsValidMockConfig) {
    EXPECT_NO_THROW(validateStartupConfig(validMockConfig()));
}

TEST(ConfigValidationTest, AcceptsPinataWithJwt) {
    AppConfig config = validMockConfig();
    config.storage.provider = "pinata";
    config.pinata.jwt = "some-jwt";
    EXPECT_NO_THROW(validateStartupConfig(config));
}

TEST(ConfigValidationTest, AcceptsPinataWithApiKeyPair) {
    AppConfig config = validMockConfig();
    config.storage.provider = "pinata";
    config.pinata.apiKey = "key";
    config.pinata.apiSecret = "secret";
    EXPECT_NO_THROW(validateStartupConfig(config));
}

TEST(ConfigValidationTest, RejectsUnknownStorageProvider) {
    AppConfig config = validMockConfig();
    config.storage.provider = "s3";
    EXPECT_THROW(validateStartupConfig(config), std::runtime_error);
}

TEST(ConfigValidationTest, RejectsPinataWithoutCredentials) {
    AppConfig config = validMockConfig();
    config.storage.provider = "pinata";
    EXPECT_THROW(validateStartupConfig(config), std::runtime_error);
}

TEST(ConfigValidationTest, RejectsPinataWithIncompleteApiKeyPair) {
    AppConfig config = validMockConfig();
    config.storage.provider = "pinata";
    config.pinata.apiKey = "key-without-secret";
    EXPECT_THROW(validateStartupConfig(config), std::runtime_error);
}

TEST(ConfigValidationTest, RejectsMissingContractAddress) {
    AppConfig config = validMockConfig();
    config.web3.contractAddress = "";
    EXPECT_THROW(validateStartupConfig(config), std::runtime_error);
}

TEST(ConfigValidationTest, RejectsMissingMinterPrivateKey) {
    AppConfig config = validMockConfig();
    config.web3.minterPrivateKey = "";
    EXPECT_THROW(validateStartupConfig(config), std::runtime_error);
}
