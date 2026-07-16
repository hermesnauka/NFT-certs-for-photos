#pragma once

#include "certificate/Certificate.h"
#include "config/Config.h"

#include <json/json.h>

// Maps Certificate entities to the CertificateDto API JSON shape, deriving explorer links.
// Mirrors app01's CertificateDtoMapper. Field names/casing must match
// docs/sdlc/05-api-design.md and the frontend's lib/types.ts exactly.
namespace nftcerts::api {

class CertificateDtoMapper {
public:
    explicit CertificateDtoMapper(config::ExplorerLinkProperties explorerLinkProperties)
        : explorerLinkProperties_(std::move(explorerLinkProperties)) {}

    Json::Value toDto(const certificate::Certificate& certificate) const;

    std::string buildEtherscanUrl(const certificate::Certificate& certificate) const;
    std::string buildOpenSeaUrl(const certificate::Certificate& certificate) const;
    std::string buildRaribleUrl(const certificate::Certificate& certificate) const;

private:
    config::ExplorerLinkProperties explorerLinkProperties_;
};

}  // namespace nftcerts::api
