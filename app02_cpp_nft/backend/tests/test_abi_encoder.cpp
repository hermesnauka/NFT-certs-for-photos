#include "blockchain/AbiEncoder.h"

#include <gtest/gtest.h>

using namespace nftcerts::blockchain;

namespace {
// Verified ground truth: real ethers v6 ABI encoding of the same call, cross-checked in-session
// against a real deployed PhotoCertificate contract (mintCertificate succeeded, status=1).
const std::string kRecipient = "0x70997970C51812dc3A010C7d01b50e0d17dc79C8";
const std::string kContentHash(64, 'a');
const std::string kExpectedCalldata =
    "0x2f0bda2c00000000000000000000000070997970c51812dc3a010c7d01b50e0d17dc79c800000000000000000000"
    "000000000000000000000000000000000000000000a0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    "aaaaaaaaaaaaaaaa00000000000000000000000070997970c51812dc3a010c7d01b50e0d17dc79c8000000000000000"
    "00000000000000000000000000000000000000000000002ee0000000000000000000000000000000000000000000"
    "000000000000000000011697066733a2f2f516d4d65746164617461000000000000000000000000000000";
}  // namespace

TEST(AbiEncoderTest, EncodesMintCertificateCallMatchingRealEthersOutput) {
    std::string encoded = encodeMintCertificateCall(kRecipient, "ipfs://QmMetadata", kContentHash, kRecipient, 750);
    EXPECT_EQ(encoded, kExpectedCalldata);
}

TEST(AbiEncoderTest, SelectorMatchesFunctionSignatureHash) {
    std::string encoded = encodeMintCertificateCall(kRecipient, "ipfs://QmMetadata", kContentHash, kRecipient, 750);
    EXPECT_EQ(encoded.substr(0, 10), "0x2f0bda2c");
}

TEST(AbiEncoderTest, DecodesTokenIdFromMatchingCertificateMintedLog) {
    EventLog log;
    log.topics = {certificateMintedEventTopic0(),
                   "0x0000000000000000000000000000000000000000000000000000000000000001",
                   "0x00000000000000000000000070997970c51812dc3a010c7d01b50e0d17dc79c8"};
    log.data = "0x";

    auto tokenId = decodeTokenIdFromLogs({log});
    ASSERT_TRUE(tokenId.has_value());
    EXPECT_EQ(*tokenId, 1u);
}

TEST(AbiEncoderTest, ReturnsEmptyWhenNoMatchingLogPresent) {
    EventLog log;
    log.topics = {"0xdeadbeef00000000000000000000000000000000000000000000000000000000"};
    log.data = "0x";

    EXPECT_FALSE(decodeTokenIdFromLogs({log}).has_value());
}

TEST(AbiEncoderTest, ReturnsEmptyForNoLogs) {
    EXPECT_FALSE(decodeTokenIdFromLogs({}).has_value());
}
