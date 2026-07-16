#include "blockchain/JsonRpcClient.h"

#include "error/Exceptions.h"
#include "net/SharedEventLoop.h"

#include <drogon/HttpClient.h>

#include <atomic>

namespace nftcerts::blockchain {

namespace {
std::atomic<int> gRequestId{1};
}

JsonRpcClient::JsonRpcClient(std::string rpcUrl) : rpcUrl_(std::move(rpcUrl)) {}

Json::Value JsonRpcClient::call(const std::string& method, const Json::Value& params) {
    auto client = drogon::HttpClient::newHttpClient(rpcUrl_, nftcerts::net::sharedClientLoop());

    Json::Value body;
    body["jsonrpc"] = "2.0";
    body["id"] = gRequestId++;
    body["method"] = method;
    body["params"] = params;

    auto req = drogon::HttpRequest::newHttpJsonRequest(body);
    req->setMethod(drogon::Post);

    auto [result, response] = client->sendRequest(req, 15.0);
    if (result != drogon::ReqResult::Ok || !response) {
        throw error::ChainUnavailableException("Could not reach Ethereum RPC endpoint: " + rpcUrl_);
    }

    auto responseJson = response->getJsonObject();
    if (!responseJson) {
        throw error::ChainUnavailableException("Ethereum RPC endpoint returned a non-JSON response: " + rpcUrl_);
    }

    if (responseJson->isMember("error") && !(*responseJson)["error"].isNull()) {
        std::string message = (*responseJson)["error"].get("message", "unknown JSON-RPC error").asString();
        throw error::MintingException("JSON-RPC error calling " + method + ": " + message);
    }

    return (*responseJson)["result"];
}

uint64_t JsonRpcClient::ethChainId() {
    Json::Value result = call("eth_chainId", Json::arrayValue);
    return std::stoull(result.asString(), nullptr, 16);
}

uint64_t JsonRpcClient::ethGetTransactionCount(const std::string& address) {
    Json::Value params(Json::arrayValue);
    params.append(address);
    params.append("latest");
    Json::Value result = call("eth_getTransactionCount", params);
    return std::stoull(result.asString(), nullptr, 16);
}

std::string JsonRpcClient::ethSendRawTransaction(const std::string& signedTxHex) {
    Json::Value params(Json::arrayValue);
    params.append(signedTxHex);
    Json::Value result = call("eth_sendRawTransaction", params);
    return result.asString();
}

Json::Value JsonRpcClient::ethGetTransactionReceipt(const std::string& txHash) {
    Json::Value params(Json::arrayValue);
    params.append(txHash);
    return call("eth_getTransactionReceipt", params);
}

std::string JsonRpcClient::ethCall(const std::string& to, const std::string& data) {
    Json::Value txObject;
    txObject["to"] = to;
    txObject["data"] = data;

    Json::Value params(Json::arrayValue);
    params.append(txObject);
    params.append("latest");

    Json::Value result = call("eth_call", params);
    return result.asString();
}

}  // namespace nftcerts::blockchain
