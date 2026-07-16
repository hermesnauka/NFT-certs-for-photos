#include "api/CertificateController.h"

#include "error/Exceptions.h"

#include <drogon/HttpAppFramework.h>

namespace nftcerts::api {

namespace {

uint64_t parseTokenId(const std::string& raw) {
    try {
        size_t consumed = 0;
        uint64_t value = std::stoull(raw, &consumed);
        if (consumed != raw.size()) {
            throw error::CertificateNotFoundException(-1);
        }
        return value;
    } catch (const std::invalid_argument&) {
        throw error::CertificateNotFoundException(-1);
    } catch (const std::out_of_range&) {
        throw error::CertificateNotFoundException(-1);
    }
}

}  // namespace

void registerCertificateRoutes(certificate::CertificateService& certificateService,
                                CertificateDtoMapper& certificateDtoMapper,
                                pdf::CertificatePdfService& certificatePdfService) {
    drogon::app().registerHandler(
        "/api/certificates/{1}",
        [&certificateService, &certificateDtoMapper](
            const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            std::string tokenIdRaw) {
            uint64_t tokenId = parseTokenId(tokenIdRaw);
            certificate::Certificate cert = certificateService.getCertificate(tokenId);
            callback(drogon::HttpResponse::newHttpJsonResponse(certificateDtoMapper.toDto(cert)));
        },
        {drogon::Get});

    drogon::app().registerHandler(
        "/api/certificates/{1}/pdf",
        [&certificateService, &certificateDtoMapper, &certificatePdfService](
            const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            std::string tokenIdRaw) {
            uint64_t tokenId = parseTokenId(tokenIdRaw);
            certificate::Certificate cert = certificateService.getCertificate(tokenId);

            std::vector<unsigned char> pdf = certificatePdfService.generate(
                cert, certificateDtoMapper.buildEtherscanUrl(cert), certificateDtoMapper.buildOpenSeaUrl(cert),
                certificateDtoMapper.buildRaribleUrl(cert));

            auto response = drogon::HttpResponse::newHttpResponse();
            response->setContentTypeCode(drogon::CT_APPLICATION_OCTET_STREAM);
            response->addHeader("Content-Type", "application/pdf");
            response->addHeader("Content-Disposition",
                                 "attachment; filename=\"certificate-" + std::to_string(tokenId) + ".pdf\"");
            response->setBody(std::string(pdf.begin(), pdf.end()));
            callback(response);
        },
        {drogon::Get});
}

}  // namespace nftcerts::api
