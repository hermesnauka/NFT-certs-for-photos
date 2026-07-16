#include "api/CertificateDtoMapper.h"

namespace nftcerts::api {

Json::Value CertificateDtoMapper::toDto(const certificate::Certificate& certificate) const {
    Json::Value dto;
    dto["tokenId"] = Json::Value(static_cast<Json::UInt64>(certificate.tokenId));
    dto["artworkId"] = certificate.artwork.id;
    dto["title"] = certificate.artwork.title;
    dto["contentHashHex"] = certificate.contentHashHex;
    dto["contractAddress"] = certificate.contractAddress;
    dto["txHash"] = certificate.txHash;
    dto["ownerAddress"] = certificate.ownerAddress;
    dto["royaltyPercentageBps"] = certificate.royaltyPercentageBps;
    dto["imageIpfsUri"] = certificate.artwork.imageIpfsUri;
    dto["metadataIpfsUri"] = certificate.artwork.metadataIpfsUri;
    dto["etherscanUrl"] = buildEtherscanUrl(certificate);
    dto["openSeaUrl"] = buildOpenSeaUrl(certificate);
    dto["raribleUrl"] = buildRaribleUrl(certificate);
    return dto;
}

std::string CertificateDtoMapper::buildEtherscanUrl(const certificate::Certificate& certificate) const {
    return explorerLinkProperties_.etherscanBaseUrl + certificate.txHash;
}

std::string CertificateDtoMapper::buildOpenSeaUrl(const certificate::Certificate& certificate) const {
    return explorerLinkProperties_.openseaBaseUrl + certificate.contractAddress + "/" +
           std::to_string(certificate.tokenId);
}

std::string CertificateDtoMapper::buildRaribleUrl(const certificate::Certificate& certificate) const {
    return explorerLinkProperties_.raribleBaseUrl + certificate.contractAddress + ":" +
           std::to_string(certificate.tokenId);
}

}  // namespace nftcerts::api
