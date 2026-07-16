#include "api/ArtistDashboardController.h"

#include <drogon/HttpAppFramework.h>

namespace nftcerts::api {

void registerArtistDashboardRoutes(certificate::CertificateService& certificateService,
                                    CertificateDtoMapper& certificateDtoMapper) {
    drogon::app().registerHandler(
        "/api/artists/{1}/dashboard",
        [&certificateService, &certificateDtoMapper](
            const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            std::string walletAddress) {
            std::vector<certificate::Certificate> certificates =
                certificateService.getCertificatesForWallet(walletAddress);

            Json::Value certsJson(Json::arrayValue);
            for (const auto& cert : certificates) {
                certsJson.append(certificateDtoMapper.toDto(cert));
            }

            Json::Value response;
            response["walletAddress"] = walletAddress;
            response["certificates"] = certsJson;
            response["totalCertificates"] = static_cast<int>(certificates.size());

            callback(drogon::HttpResponse::newHttpJsonResponse(response));
        },
        {drogon::Get});
}

}  // namespace nftcerts::api
