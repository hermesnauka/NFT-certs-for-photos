#include "config/Config.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace nftcerts::config {

namespace {

// Minimal .env loader: KEY=VALUE lines, '#' comments, no quoting/escaping needed for this
// project's values. Only sets a variable if it is not already present in the environment, so
// real environment variables always win over a checked-in .env file (matches common .env
// convention and avoids surprising a deployment that sets real env vars).
void loadDotEnv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        auto hash = line.find('#');
        if (hash != std::string::npos) {
            line = line.substr(0, hash);
        }
        auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        auto trim = [](std::string& s) {
            size_t start = s.find_first_not_of(" \t\r\n");
            size_t end = s.find_last_not_of(" \t\r\n");
            s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
        };
        trim(key);
        trim(value);
        if (key.empty()) {
            continue;
        }
        setenv(key.c_str(), value.c_str(), /*overwrite=*/0);
    }
}

std::string envOr(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value != nullptr) ? std::string(value) : fallback;
}

}  // namespace

AppConfig AppConfig::loadFromEnv() {
    loadDotEnv(".env");
    loadDotEnv("../.env");

    AppConfig config;

    config.serverPort = std::stoi(envOr("SERVER_PORT", "8081"));
    config.dbPath = envOr("DB_PATH", "data/nft-certs.db");

    config.storage.provider = envOr("APP_STORAGE_PROVIDER", "pinata");

    config.pinata.jwt = envOr("PINATA_JWT", "");
    config.pinata.apiKey = envOr("PINATA_API_KEY", "");
    config.pinata.apiSecret = envOr("PINATA_API_SECRET", "");
    config.pinata.baseUrl = envOr("PINATA_BASE_URL", "https://api.pinata.cloud");

    config.web3.rpcUrl = envOr("WEB3J_RPC_URL", "http://localhost:8545");
    config.web3.minterPrivateKey = envOr("MINTER_PRIVATE_KEY", "");
    config.web3.contractAddress = envOr("NFT_CONTRACT_ADDRESS", "");

    config.explorer.etherscanBaseUrl = envOr("ETHERSCAN_BASE_URL", "http://localhost:8545/tx/");
    config.explorer.openseaBaseUrl = envOr("OPENSEA_BASE_URL", "https://testnets.opensea.io/assets/");
    config.explorer.raribleBaseUrl = envOr("RARIBLE_BASE_URL", "https://rarible.com/token/");

    return config;
}

void validateStartupConfig(const AppConfig& config) {
    if (!config.storage.isMock() && !config.storage.isPinata()) {
        throw std::runtime_error(
            "Invalid APP_STORAGE_PROVIDER '" + config.storage.provider + "': must be 'pinata' or 'mock'");
    }

    if (config.storage.isPinata() && !config.pinata.hasJwt() && !config.pinata.hasApiKeyPair()) {
        throw std::runtime_error(
            "Missing Pinata credentials: set PINATA_JWT, or both PINATA_API_KEY and PINATA_API_SECRET "
            "(or set APP_STORAGE_PROVIDER=mock to run without Pinata)");
    }
    if (config.web3.contractAddress.empty()) {
        throw std::runtime_error("Missing required environment variable: NFT_CONTRACT_ADDRESS");
    }
    if (config.web3.minterPrivateKey.empty()) {
        throw std::runtime_error("Missing required environment variable: MINTER_PRIVATE_KEY");
    }
}

}  // namespace nftcerts::config
