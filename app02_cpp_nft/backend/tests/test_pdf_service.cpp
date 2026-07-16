#include "pdf/CertificatePdfService.h"

#include <gtest/gtest.h>

#include <string>

using namespace nftcerts;

namespace {

certificate::Certificate sampleCertificate() {
    certificate::Certificate cert;
    cert.tokenId = 42;
    cert.artwork.id = "artwork-1";
    cert.artwork.title = "Morning Fog";
    cert.artwork.medium = "Photography";
    cert.artwork.yearCreated = 2024;
    cert.artwork.artistDid = "did:key:z6MkTest";
    cert.artwork.imageIpfsUri = "ipfs://QmImage";
    cert.artwork.metadataIpfsUri = "ipfs://QmMeta";
    cert.contentHashHex = std::string(64, 'a');
    cert.contractAddress = "0x5FbDB2315678afecb367f032d93F642f64180aa3";
    cert.txHash = "0x" + std::string(64, 'b');
    cert.ownerAddress = "0x70997970C51812dc3A010C7d01b50e0d17dc79C8";
    cert.royaltyPercentageBps = 500;
    cert.royaltyRecipient = "0x70997970C51812dc3A010C7d01b50e0d17dc79C8";
    cert.mintedAt = "2026-01-01T12:00:00Z";
    return cert;
}

}  // namespace

TEST(CertificatePdfServiceTest, GeneratesValidPdfBytes) {
    pdf::CertificatePdfService service;
    std::vector<unsigned char> bytes =
        service.generate(sampleCertificate(), "http://etherscan.test/tx/0xb", "http://opensea.test/a/42",
                          "http://rarible.test/a:42");

    ASSERT_GT(bytes.size(), 500u);
    std::string prefix(bytes.begin(), bytes.begin() + 5);
    EXPECT_EQ(prefix, "%PDF-");
    // A complete PDF ends with %%EOF (possibly followed by a newline).
    std::string tail(bytes.end() - std::min<size_t>(bytes.size(), 16), bytes.end());
    EXPECT_NE(tail.find("%%EOF"), std::string::npos);
}

TEST(CertificatePdfServiceTest, HandlesMissingOptionalFields) {
    certificate::Certificate cert = sampleCertificate();
    cert.artwork.title = "";
    cert.artwork.medium = "";
    cert.artwork.yearCreated = std::nullopt;

    pdf::CertificatePdfService service;
    std::vector<unsigned char> bytes = service.generate(cert, "", "", "");
    ASSERT_GT(bytes.size(), 500u);
    EXPECT_EQ(std::string(bytes.begin(), bytes.begin() + 5), "%PDF-");
}
