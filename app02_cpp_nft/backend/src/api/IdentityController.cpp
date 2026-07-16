#include "api/IdentityController.h"

#include "error/Exceptions.h"

#include <drogon/HttpAppFramework.h>

#include <regex>

namespace nftcerts::api {

namespace {
const std::regex kWalletAddressPattern("^0x[a-fA-F0-9]{40}$");
const std::regex kDidPattern("^did:[a-zA-Z0-9]+:.+$");
}  // namespace

void registerIdentityRoutes(identity::KycVerificationService& kycVerificationService) {
    drogon::app().registerHandler(
        "/api/identity/verify",
        [&kycVerificationService](const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            auto body = req->getJsonObject();
            if (!body || !body->isMember("walletAddress") || !body->isMember("did")) {
                throw error::MalformedIdentityException("walletAddress and did are required");
            }
            std::string walletAddress = (*body)["walletAddress"].asString();
            std::string did = (*body)["did"].asString();
            std::string email = body->get("email", "").asString();

            if (!std::regex_match(walletAddress, kWalletAddressPattern)) {
                throw error::MalformedIdentityException("Malformed wallet address: " + walletAddress);
            }
            if (!std::regex_match(did, kDidPattern)) {
                throw error::MalformedIdentityException("Malformed DID: " + did);
            }

            identity::ArtistIdentity identityRecord = kycVerificationService.verify(walletAddress, did, email);

            Json::Value response;
            response["verified"] = identityRecord.verified;
            response["did"] = identityRecord.did;
            response["walletAddress"] = identityRecord.walletAddress;

            callback(drogon::HttpResponse::newHttpJsonResponse(response));
        },
        {drogon::Post});
}

}  // namespace nftcerts::api
