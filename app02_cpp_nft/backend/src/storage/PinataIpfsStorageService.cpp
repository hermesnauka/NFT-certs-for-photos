#include "storage/PinataIpfsStorageService.h"

#include "error/Exceptions.h"
#include "net/SharedEventLoop.h"
#include "util/Uuid.h"

#include <drogon/HttpClient.h>

#include <sstream>

namespace nftcerts::storage {

namespace {

constexpr const char* kPinFilePath = "/pinning/pinFileToIPFS";
constexpr const char* kPinJsonPath = "/pinning/pinJSONToIPFS";

}  // namespace

PinataIpfsStorageService::PinataIpfsStorageService(config::PinataProperties pinataProperties)
    : pinataProperties_(std::move(pinataProperties)) {}

PinResult PinataIpfsStorageService::pinFile(const std::vector<unsigned char>& content, const std::string& filename,
                                             const std::string& contentType) {
    std::string boundary = "----NftCertsBoundary" + util::randomUuid();

    std::string body;
    body += "--" + boundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n";
    body += "Content-Type: " + (contentType.empty() ? std::string("application/octet-stream") : contentType) +
            "\r\n\r\n";
    body.append(reinterpret_cast<const char*>(content.data()), content.size());
    body += "\r\n--" + boundary + "--\r\n";

    auto client = drogon::HttpClient::newHttpClient(pinataProperties_.baseUrl, nftcerts::net::sharedClientLoop());
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath(kPinFilePath);
    req->addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    if (pinataProperties_.hasJwt()) {
        req->addHeader("Authorization", "Bearer " + pinataProperties_.jwt);
    } else if (pinataProperties_.hasApiKeyPair()) {
        req->addHeader("pinata_api_key", pinataProperties_.apiKey);
        req->addHeader("pinata_secret_api_key", pinataProperties_.apiSecret);
    } else {
        throw error::IpfsPinningException(
            "No Pinata credentials configured (PINATA_JWT or PINATA_API_KEY/PINATA_API_SECRET)");
    }
    req->setBody(body);

    auto [result, response] = client->sendRequest(req, 30.0);
    if (result != drogon::ReqResult::Ok || !response) {
        throw error::IpfsPinningException("Failed to pin file to IPFS via Pinata: request failed");
    }
    if (response->getStatusCode() < 200 || response->getStatusCode() >= 300) {
        throw error::IpfsPinningException("Failed to pin file to IPFS via Pinata: HTTP " +
                                           std::to_string(response->getStatusCode()));
    }

    return PinResult::of(extractCid(std::string(response->getBody())));
}

PinResult PinataIpfsStorageService::pinJson(const Json::Value& jsonPayload, const std::string& name) {
    Json::Value wrapper;
    Json::Value metadata;
    metadata["name"] = name;
    wrapper["pinataMetadata"] = metadata;
    wrapper["pinataContent"] = jsonPayload;

    auto client = drogon::HttpClient::newHttpClient(pinataProperties_.baseUrl, nftcerts::net::sharedClientLoop());
    auto req = drogon::HttpRequest::newHttpJsonRequest(wrapper);
    req->setMethod(drogon::Post);
    req->setPath(kPinJsonPath);

    if (pinataProperties_.hasJwt()) {
        req->addHeader("Authorization", "Bearer " + pinataProperties_.jwt);
    } else if (pinataProperties_.hasApiKeyPair()) {
        req->addHeader("pinata_api_key", pinataProperties_.apiKey);
        req->addHeader("pinata_secret_api_key", pinataProperties_.apiSecret);
    } else {
        throw error::IpfsPinningException(
            "No Pinata credentials configured (PINATA_JWT or PINATA_API_KEY/PINATA_API_SECRET)");
    }

    auto [result, response] = client->sendRequest(req, 30.0);
    if (result != drogon::ReqResult::Ok || !response) {
        throw error::IpfsPinningException("Failed to pin JSON to IPFS via Pinata: request failed");
    }
    if (response->getStatusCode() < 200 || response->getStatusCode() >= 300) {
        throw error::IpfsPinningException("Failed to pin JSON to IPFS via Pinata: HTTP " +
                                           std::to_string(response->getStatusCode()));
    }

    return PinResult::of(extractCid(std::string(response->getBody())));
}

std::string PinataIpfsStorageService::extractCid(const std::string& responseBody) {
    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errs;
    std::istringstream stream(responseBody);
    if (!Json::parseFromStream(builder, stream, &root, &errs)) {
        throw error::IpfsPinningException("Failed to parse Pinata response: " + responseBody);
    }
    if (!root.isMember("IpfsHash") || root["IpfsHash"].asString().empty()) {
        throw error::IpfsPinningException("Pinata response did not contain an IpfsHash: " + responseBody);
    }
    return root["IpfsHash"].asString();
}

}  // namespace nftcerts::storage
