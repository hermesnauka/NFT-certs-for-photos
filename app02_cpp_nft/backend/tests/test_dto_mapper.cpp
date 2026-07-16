#include "api/CertificateDtoMapper.h"

#include <gtest/gtest.h>

using namespace nftcerts;

namespace {

certificate::Certificate sampleCertificate() {
    certificate::Certificate cert;
    cert.tokenId = 7;
    cert.artwork.id = "artwork-1";
    cert.artwork.title = "Sunset";
    cert.artwork.imageIpfsUri = "ipfs://QmImage";
    cert.artwork.metadataIpfsUri = "ipfs://QmMeta";
    cert.contentHashHex = std::string(64, 'c');
    cert.contractAddress = "0xcontract";
    cert.txHash = "0xtxhash";
    cert.ownerAddress = "0xowner";
    cert.royaltyPercentageBps = 750;
    cert.royaltyRecipient = "0xroyalty";
    cert.mintedAt = "2026-01-01T00:00:00Z";
    return cert;
}

config::ExplorerLinkProperties testExplorerProps() {
    config::ExplorerLinkProperties props;
    props.etherscanBaseUrl = "http://etherscan.test/tx/";
    props.openseaBaseUrl = "http://opensea.test/assets/";
    props.raribleBaseUrl = "http://rarible.test/token/";
    return props;
}

}  // namespace

TEST(CertificateDtoMapperTest, MapsAllDtoFields) {
    api::CertificateDtoMapper mapper(testExplorerProps());
    Json::Value dto = mapper.toDto(sampleCertificate());

    EXPECT_EQ(dto["tokenId"].asUInt64(), 7u);
    EXPECT_EQ(dto["artworkId"].asString(), "artwork-1");
    EXPECT_EQ(dto["title"].asString(), "Sunset");
    EXPECT_EQ(dto["contentHashHex"].asString(), std::string(64, 'c'));
    EXPECT_EQ(dto["contractAddress"].asString(), "0xcontract");
    EXPECT_EQ(dto["txHash"].asString(), "0xtxhash");
    EXPECT_EQ(dto["ownerAddress"].asString(), "0xowner");
    EXPECT_EQ(dto["royaltyPercentageBps"].asInt(), 750);
    EXPECT_EQ(dto["imageIpfsUri"].asString(), "ipfs://QmImage");
    EXPECT_EQ(dto["metadataIpfsUri"].asString(), "ipfs://QmMeta");
}

TEST(CertificateDtoMapperTest, BuildsExplorerUrls) {
    api::CertificateDtoMapper mapper(testExplorerProps());
    certificate::Certificate cert = sampleCertificate();

    EXPECT_EQ(mapper.buildEtherscanUrl(cert), "http://etherscan.test/tx/0xtxhash");
    EXPECT_EQ(mapper.buildOpenSeaUrl(cert), "http://opensea.test/assets/0xcontract/7");
    EXPECT_EQ(mapper.buildRaribleUrl(cert), "http://rarible.test/token/0xcontract:7");

    Json::Value dto = mapper.toDto(cert);
    EXPECT_EQ(dto["etherscanUrl"].asString(), "http://etherscan.test/tx/0xtxhash");
    EXPECT_EQ(dto["openSeaUrl"].asString(), "http://opensea.test/assets/0xcontract/7");
    EXPECT_EQ(dto["raribleUrl"].asString(), "http://rarible.test/token/0xcontract:7");
}
