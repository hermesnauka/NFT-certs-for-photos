#pragma once

#include <string>

namespace nftcerts::config {

// Mirrors app01's PinataProperties (pinata.*).
struct PinataProperties {
    std::string jwt;
    std::string apiKey;
    std::string apiSecret;
    std::string baseUrl = "https://api.pinata.cloud";

    bool hasJwt() const { return !jwt.empty(); }
    bool hasApiKeyPair() const { return !apiKey.empty() && !apiSecret.empty(); }
};

// Mirrors app01's Web3Properties (web3.*).
struct Web3Properties {
    std::string rpcUrl = "http://localhost:8545";
    std::string minterPrivateKey;
    std::string contractAddress;
};

// Mirrors app01's ExplorerLinkProperties (explorer.*).
struct ExplorerLinkProperties {
    std::string etherscanBaseUrl = "http://localhost:8545/tx/";
    std::string openseaBaseUrl = "https://testnets.opensea.io/assets/";
    std::string raribleBaseUrl = "https://rarible.com/token/";
};

// Mirrors app01's StorageProperties (app.storage.*).
struct StorageProperties {
    std::string provider = "pinata";

    bool isMock() const { return provider == "mock"; }
    bool isPinata() const { return provider == "pinata"; }
};

struct AppConfig {
    int serverPort = 8081;
    std::string dbPath = "data/nft-certs.db";
    PinataProperties pinata;
    Web3Properties web3;
    ExplorerLinkProperties explorer;
    StorageProperties storage;

    // Reads from environment variables, matching app01's application.yml env-var bindings.
    // Loads a .env file first (if present) into the process environment, then reads getenv().
    static AppConfig loadFromEnv();
};

// Throws std::runtime_error with a descriptive message if required configuration is missing,
// mirroring StartupConfigurationValidator. Skipped entirely when skipValidation is true (used by
// the mock/offline test profile equivalent).
void validateStartupConfig(const AppConfig& config);

}  // namespace nftcerts::config
