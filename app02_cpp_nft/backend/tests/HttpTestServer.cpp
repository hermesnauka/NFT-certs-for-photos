#include "HttpTestServer.h"

#include "api/ArtistDashboardController.h"
#include "api/ArtworkController.h"
#include "api/CertificateController.h"
#include "api/CertificateDtoMapper.h"
#include "api/ExceptionMapping.h"
#include "api/I18nController.h"
#include "api/IdentityController.h"
#include "api/UploadController.h"
#include "api/UploadStore.h"
#include "blockchain/AbiEncoder.h"
#include "blockchain/ContractService.h"
#include "certificate/ArtworkRepository.h"
#include "certificate/CertificateRepository.h"
#include "certificate/CertificateService.h"
#include "db/Database.h"
#include "hashing/Sha256HashingService.h"
#include "identity/ArtistIdentityRepository.h"
#include "identity/MockKycVerificationService.h"
#include "net/SharedEventLoop.h"
#include "pdf/CertificatePdfService.h"
#include "storage/LocalStubIpfsStorageService.h"
#include "watermark/MetadataWatermarkService.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace nftcerts::testsupport {

HttpTestServer::RpcOverride HttpTestServer::rpcOverride = nullptr;
std::atomic<int> HttpTestServer::pinataStatusOverride{0};
std::atomic<bool> HttpTestServer::pinataOmitIpfsHash{false};
std::map<std::string, std::string> HttpTestServer::lastPinataHeaders;

void HttpTestServer::resetPinataBehavior() {
    pinataStatusOverride = 0;
    pinataOmitIpfsHash = false;
    lastPinataHeaders.clear();
}

const char* HttpTestServer::contractAddress() { return "0x5fbdb2315678afecb367f032d93f642f64180aa3"; }

namespace {

// Hardhat's well-known test account #0 key — never a real-funds key.
constexpr const char* kMinterKey = "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80";

// Binds an ephemeral port, closes the socket, and returns the port number. The tiny window
// between close and Drogon's own bind is acceptable for tests.
int findFreePort() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("socket() failed");
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        ::close(fd);
        throw std::runtime_error("bind() failed");
    }
    socklen_t len = sizeof(addr);
    if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
        ::close(fd);
        throw std::runtime_error("getsockname() failed");
    }
    int port = ntohs(addr.sin_port);
    ::close(fd);
    return port;
}

// All services live here with static storage duration so route lambdas can capture references
// that stay valid for the whole test process.
struct Services {
    db::Database db{":memory:"};
    hashing::Sha256HashingService hashingService;
    watermark::MetadataWatermarkService watermarkService;
    pdf::CertificatePdfService pdfService;
    certificate::ArtworkRepository artworkRepository{db};
    certificate::CertificateRepository certificateRepository{db};
    identity::ArtistIdentityRepository artistIdentityRepository{db};
    identity::MockKycVerificationService kycService{artistIdentityRepository};
    storage::LocalStubIpfsStorageService storageService;
    std::unique_ptr<blockchain::ContractService> contractService;  // needs the port; built later
    std::unique_ptr<certificate::CertificateService> certificateService;
    api::UploadStore uploadStore;
    std::unique_ptr<api::CertificateDtoMapper> dtoMapper;

    Services() { db::initSchema(db); }
};

Services& services() {
    static Services instance;
    return instance;
}

std::string tokenIdTopic(uint64_t tokenId) {
    std::ostringstream hex;
    hex << std::hex << tokenId;
    std::string raw = hex.str();
    return "0x" + std::string(64 - raw.size(), '0') + raw;
}

void registerFakeJsonRpc() {
    drogon::app().registerHandler(
        "/",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            auto body = req->getJsonObject();
            Json::Value reply;
            reply["jsonrpc"] = "2.0";
            reply["id"] = body ? (*body)["id"] : Json::Value(0);

            std::string method = body ? (*body)["method"].asString() : "";
            Json::Value params = body ? (*body)["params"] : Json::Value(Json::arrayValue);

            Json::Value result;
            std::string errorMessage;
            bool handled = false;
            if (HttpTestServer::rpcOverride) {
                handled = HttpTestServer::rpcOverride(method, params, result, errorMessage);
            }

            if (!handled) {
                // Default happy-path chain: every sendRawTransaction mints the next tokenId and
                // its receipt is immediately available with a matching CertificateMinted log.
                static std::atomic<uint64_t> nextTokenId{0};
                static std::atomic<uint64_t> lastTokenId{0};
                if (method == "eth_chainId") {
                    result = "0x7a69";  // 31337
                } else if (method == "eth_getTransactionCount") {
                    result = "0x0";
                } else if (method == "eth_sendRawTransaction") {
                    lastTokenId = ++nextTokenId;
                    result = "0xfaketxhash" + std::to_string(lastTokenId.load());
                } else if (method == "eth_getTransactionReceipt") {
                    result = Json::Value(Json::objectValue);
                    result["status"] = "0x1";
                    Json::Value log;
                    Json::Value topics(Json::arrayValue);
                    topics.append(blockchain::certificateMintedEventTopic0());
                    topics.append(tokenIdTopic(lastTokenId.load()));
                    topics.append("0x0000000000000000000000007099" + std::string(36, '0'));
                    log["topics"] = topics;
                    log["data"] = "0x";
                    Json::Value logs(Json::arrayValue);
                    logs.append(log);
                    result["logs"] = logs;
                } else if (method == "eth_call") {
                    result = "0x";
                } else {
                    errorMessage = "unsupported fake RPC method: " + method;
                }
            }

            if (!errorMessage.empty()) {
                Json::Value error;
                error["code"] = -32000;
                error["message"] = errorMessage;
                reply["error"] = error;
            } else {
                reply["result"] = result;
            }
            callback(drogon::HttpResponse::newHttpJsonResponse(reply));
        },
        {drogon::Post});
}

void registerFakePinata() {
    auto handler = [](const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
        HttpTestServer::lastPinataHeaders.clear();
        for (const char* name : {"authorization", "pinata_api_key", "pinata_secret_api_key"}) {
            std::string value = req->getHeader(name);
            if (!value.empty()) HttpTestServer::lastPinataHeaders[name] = value;
        }

        int statusOverride = HttpTestServer::pinataStatusOverride.load();
        if (statusOverride != 0) {
            auto response = drogon::HttpResponse::newHttpResponse();
            response->setStatusCode(static_cast<drogon::HttpStatusCode>(statusOverride));
            callback(response);
            return;
        }

        Json::Value body;
        if (!HttpTestServer::pinataOmitIpfsHash.load()) {
            body["IpfsHash"] = "QmFakePinataCid";
        }
        body["PinSize"] = 1234;
        callback(drogon::HttpResponse::newHttpJsonResponse(body));
    };
    drogon::app().registerHandler("/pinning/pinFileToIPFS", handler, {drogon::Post});
    drogon::app().registerHandler("/pinning/pinJSONToIPFS", handler, {drogon::Post});
}

}  // namespace

HttpTestServer& HttpTestServer::instance() {
    static HttpTestServer server;
    return server;
}

HttpTestServer::HttpTestServer() {
    int port = findFreePort();
    baseUrl_ = "http://127.0.0.1:" + std::to_string(port);

    Services& s = services();
    config::Web3Properties web3;
    web3.rpcUrl = baseUrl_;
    web3.minterPrivateKey = kMinterKey;
    web3.contractAddress = contractAddress();
    s.contractService = std::make_unique<blockchain::ContractService>(web3);
    s.certificateService = std::make_unique<certificate::CertificateService>(
        s.artworkRepository, s.certificateRepository, s.storageService, *s.contractService, s.kycService);

    config::ExplorerLinkProperties explorer;
    explorer.etherscanBaseUrl = "http://etherscan.test/tx/";
    explorer.openseaBaseUrl = "http://opensea.test/assets/";
    explorer.raribleBaseUrl = "http://rarible.test/token/";
    s.dtoMapper = std::make_unique<api::CertificateDtoMapper>(explorer);

    api::registerExceptionHandler();
    api::registerUploadRoutes(s.hashingService, s.watermarkService, s.uploadStore);
    api::registerArtworkRoutes(s.uploadStore, *s.certificateService, *s.dtoMapper);
    api::registerCertificateRoutes(*s.certificateService, *s.dtoMapper, s.pdfService);
    api::registerArtistDashboardRoutes(*s.certificateService, *s.dtoMapper);
    api::registerI18nRoutes();
    api::registerIdentityRoutes(s.kycService);
    registerFakeJsonRpc();
    registerFakePinata();

    drogon::app().addListener("127.0.0.1", static_cast<uint16_t>(port));
    // Multiple IO threads so a handler blocked on a nested sync HTTP call (mint -> fake RPC on
    // this same server) can be served by another thread instead of deadlocking.
    drogon::app().setThreadNum(4);
    drogon::app().setLogLevel(trantor::Logger::kWarn);
    drogon::app().disableSigtermHandling();

    // The app runs until process exit; no clean shutdown is attempted because Drogon teardown is
    // not static-destruction-safe — tests/test_main.cpp ends the process with _Exit instead.
    std::thread([] { drogon::app().run(); }).detach();

    // Wait until the server actually answers.
    for (int attempt = 0; attempt < 200; ++attempt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
        if (!drogon::app().isRunning()) continue;
        try {
            auto probe = drogon::HttpRequest::newHttpRequest();
            probe->setPath("/api/i18n/en");
            auto client = drogon::HttpClient::newHttpClient(baseUrl_, net::sharedClientLoop());
            auto [result, response] = client->sendRequest(probe, 2.0);
            if (result == drogon::ReqResult::Ok && response &&
                response->getStatusCode() == drogon::k200OK) {
                return;
            }
        } catch (...) {
        }
    }
    throw std::runtime_error("HttpTestServer failed to start on " + baseUrl_);
}

HttpTestServer::~HttpTestServer() = default;

drogon::HttpResponsePtr HttpTestServer::sendSync(const drogon::HttpRequestPtr& request) const {
    auto client = drogon::HttpClient::newHttpClient(baseUrl_, net::sharedClientLoop());
    auto [result, response] = client->sendRequest(request, 60.0);
    if (result != drogon::ReqResult::Ok || !response) {
        throw std::runtime_error("test request failed against " + baseUrl_);
    }
    return response;
}

drogon::HttpResponsePtr HttpTestServer::get(const std::string& path) const {
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setPath(path);
    return sendSync(request);
}

drogon::HttpResponsePtr HttpTestServer::postJson(const std::string& path, const Json::Value& body) const {
    auto request = drogon::HttpRequest::newHttpJsonRequest(body);
    request->setMethod(drogon::Post);
    request->setPath(path);
    return sendSync(request);
}

drogon::HttpResponsePtr HttpTestServer::postMultipartFile(const std::string& path, const std::string& filename,
                                                           const std::string& content) const {
    // Drogon's multipart request builder only reads from disk, so stage the bytes in a temp file.
    static std::atomic<int> fileCounter{0};
    std::filesystem::path tempPath =
        std::filesystem::temp_directory_path() / ("nftcerts-test-upload-" + std::to_string(::getpid()) + "-" +
                                                   std::to_string(fileCounter++) + "-" + filename);
    {
        std::ofstream out(tempPath, std::ios::binary);
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
    }

    drogon::UploadFile file(tempPath.string(), filename, "file");
    auto request = drogon::HttpRequest::newFileUploadRequest({file});
    request->setMethod(drogon::Post);
    request->setPath(path);
    auto response = sendSync(request);
    std::filesystem::remove(tempPath);
    return response;
}

}  // namespace nftcerts::testsupport
