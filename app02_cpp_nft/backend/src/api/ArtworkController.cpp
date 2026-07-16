#include "api/ArtworkController.h"

#include "error/Exceptions.h"

#include <drogon/HttpAppFramework.h>

namespace nftcerts::api {

namespace {

std::string requireString(const Json::Value& body, const std::string& field, bool allowBlank = false) {
    if (!body.isMember(field) || !body[field].isString()) {
        if (allowBlank) return "";
        throw error::ValidationException(field + ": must not be blank");
    }
    std::string value = body[field].asString();
    if (value.empty() && !allowBlank) {
        throw error::ValidationException(field + ": must not be blank");
    }
    return value;
}

}  // namespace

void registerArtworkRoutes(UploadStore& uploadStore, certificate::CertificateService& certificateService,
                            CertificateDtoMapper& certificateDtoMapper) {
    drogon::app().registerHandler(
        "/api/artworks",
        [&uploadStore, &certificateService](const drogon::HttpRequestPtr& req,
                                             std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            auto body = req->getJsonObject();
            if (!body) {
                throw error::ValidationException("Request body must be valid JSON");
            }

            std::string fileId = requireString(*body, "fileId");
            std::string title = requireString(*body, "title");
            std::string description = body->get("description", "").asString();
            std::string medium = body->get("medium", "").asString();
            std::optional<int> yearCreated;
            if (body->isMember("yearCreated") && (*body)["yearCreated"].isInt()) {
                yearCreated = (*body)["yearCreated"].asInt();
            }
            std::optional<int> royaltyPercentageBps;
            if (body->isMember("royaltyPercentageBps") && (*body)["royaltyPercentageBps"].isInt()) {
                royaltyPercentageBps = (*body)["royaltyPercentageBps"].asInt();
            } else {
                throw error::ValidationException("royaltyPercentageBps: must not be null");
            }
            std::string artistWalletAddress = requireString(*body, "artistWalletAddress");
            std::string artistDid = body->get("artistDid", "").asString();

            certificate::StoredUpload upload = uploadStore.get(fileId);

            certificate::Artwork artwork = certificateService.createArtwork(
                upload, title, description, medium, yearCreated, royaltyPercentageBps, artistWalletAddress,
                artistDid);

            Json::Value response;
            response["artworkId"] = artwork.id;
            response["sha256Hash"] = artwork.sha256Hash;
            response["imageIpfsUri"] = artwork.imageIpfsUri;
            response["metadataIpfsUri"] = artwork.metadataIpfsUri;
            response["status"] = toString(artwork.status);

            auto httpResponse = drogon::HttpResponse::newHttpJsonResponse(response);
            httpResponse->setStatusCode(drogon::k201Created);
            callback(httpResponse);
        },
        {drogon::Post});

    drogon::app().registerHandler(
        "/api/artworks/{1}/mint",
        [&certificateService, &certificateDtoMapper](
            const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            std::string artworkId) {
            auto body = req->getJsonObject();
            if (!body) {
                throw error::ValidationException("Request body must be valid JSON");
            }
            std::string recipientAddress = requireString(*body, "recipientAddress");

            certificate::Certificate cert = certificateService.mintCertificate(artworkId, recipientAddress);
            callback(drogon::HttpResponse::newHttpJsonResponse(certificateDtoMapper.toDto(cert)));
        },
        {drogon::Post});
}

}  // namespace nftcerts::api
