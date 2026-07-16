#pragma once

#include <json/json.h>

#include <string>
#include <vector>

// Minimal synchronous Ethereum JSON-RPC client (eth_chainId, eth_getTransactionCount,
// eth_sendRawTransaction, eth_getTransactionReceipt, eth_call), built on Drogon's HttpClient.
// Mirrors the subset of Web3j's Web3j interface that app01's ContractService actually uses.
namespace nftcerts::blockchain {

class JsonRpcClient {
public:
    explicit JsonRpcClient(std::string rpcUrl);

    uint64_t ethChainId();
    uint64_t ethGetTransactionCount(const std::string& address);

    // Returns the transaction hash, or throws on a JSON-RPC error (e.g. "execution reverted:
    // ...") — the caller inspects the error message for the duplicate-content-hash revert reason.
    std::string ethSendRawTransaction(const std::string& signedTxHex);

    // Returns the raw JSON receipt object, or Json::nullValue if not yet mined.
    Json::Value ethGetTransactionReceipt(const std::string& txHash);

    // Returns the raw hex-encoded return data ("0x...").
    std::string ethCall(const std::string& to, const std::string& data);

private:
    Json::Value call(const std::string& method, const Json::Value& params);

    std::string rpcUrl_;
};

}  // namespace nftcerts::blockchain
