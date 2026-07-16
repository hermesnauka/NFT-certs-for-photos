#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <json/json.h>

#include <atomic>
#include <functional>
#include <map>
#include <string>

// Shared in-process Drogon server for HTTP-level tests. Started lazily (once per test process)
// on a free localhost port, it hosts:
//   - every production API route, wired to an in-memory SQLite DB, LocalStubIpfsStorageService,
//     mock KYC, and a ContractService whose JSON-RPC URL points back at this same server;
//   - a fake Ethereum JSON-RPC endpoint at POST / (default: happy-path mint with an
//     auto-incrementing tokenId; override per-test via `rpcOverride`);
//   - a fake Pinata API at /pinning/pinFileToIPFS and /pinning/pinJSONToIPFS (failure modes via
//     `pinataStatusOverride` / `pinataOmitIpfsHash`).
//
// drogon::app() is a process-wide singleton, so there is exactly one server per test process;
// ctest (gtest_discover_tests) runs each test case in its own process, which keeps tests isolated.
namespace nftcerts::testsupport {

class HttpTestServer {
public:
    // Starts the server on first call; subsequent calls return the same instance.
    static HttpTestServer& instance();

    const std::string& baseUrl() const { return baseUrl_; }

    // Sends a request to the test server synchronously (10 s timeout, throws on transport error).
    drogon::HttpResponsePtr sendSync(const drogon::HttpRequestPtr& request) const;

    // Convenience wrappers.
    drogon::HttpResponsePtr get(const std::string& path) const;
    drogon::HttpResponsePtr postJson(const std::string& path, const Json::Value& body) const;
    // Builds and sends a multipart/form-data upload of `content` as field "file".
    drogon::HttpResponsePtr postMultipartFile(const std::string& path, const std::string& filename,
                                               const std::string& content) const;

    // --- fake JSON-RPC control -------------------------------------------------------------
    // If set, called for every JSON-RPC request before the default behavior. Return true and
    // fill `result` (or `errorMessage` for a JSON-RPC error object) to handle the call yourself.
    using RpcOverride = std::function<bool(const std::string& method, const Json::Value& params,
                                            Json::Value& result, std::string& errorMessage)>;
    static RpcOverride rpcOverride;
    static void resetRpcOverride() { rpcOverride = nullptr; }

    // --- fake Pinata control ---------------------------------------------------------------
    static std::atomic<int> pinataStatusOverride;  // 0 = respond 200; else respond this status
    static std::atomic<bool> pinataOmitIpfsHash;   // true = 200 body without "IpfsHash"
    static std::map<std::string, std::string> lastPinataHeaders;  // auth headers of last request
    static void resetPinataBehavior();

    // Contract address the built-in ContractService is configured with.
    static const char* contractAddress();

    HttpTestServer(const HttpTestServer&) = delete;
    HttpTestServer& operator=(const HttpTestServer&) = delete;

private:
    HttpTestServer();
    ~HttpTestServer();

    std::string baseUrl_;
};

}  // namespace nftcerts::testsupport
