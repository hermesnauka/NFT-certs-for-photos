#include "blockchain/Hex.h"

#include <gtest/gtest.h>

using namespace nftcerts::blockchain;

TEST(HexTest, StripHexPrefix) {
    EXPECT_EQ(stripHexPrefix("0xabcd"), "abcd");
    EXPECT_EQ(stripHexPrefix("0Xabcd"), "abcd");
    EXPECT_EQ(stripHexPrefix("abcd"), "abcd");
    EXPECT_EQ(stripHexPrefix("0x"), "");
    EXPECT_EQ(stripHexPrefix(""), "");
}

TEST(HexTest, HexToBytesWithAndWithoutPrefix) {
    std::vector<uint8_t> expected{0xde, 0xad, 0xbe, 0xef};
    EXPECT_EQ(hexToBytes("0xdeadbeef"), expected);
    EXPECT_EQ(hexToBytes("deadbeef"), expected);
}

TEST(HexTest, HexToBytesPadsOddLengthWithLeadingZero) {
    EXPECT_EQ(hexToBytes("0xf"), (std::vector<uint8_t>{0x0f}));
    EXPECT_EQ(hexToBytes("abc"), (std::vector<uint8_t>{0x0a, 0xbc}));
}

TEST(HexTest, BytesToHexRoundTrip) {
    std::vector<uint8_t> bytes{0x00, 0x01, 0xff, 0x7a};
    EXPECT_EQ(bytesToHex(bytes), "0x0001ff7a");
    EXPECT_EQ(bytesToHex(bytes, /*withPrefix=*/false), "0001ff7a");
    EXPECT_EQ(hexToBytes(bytesToHex(bytes)), bytes);
    EXPECT_EQ(bytesToHex({}), "0x");
}

TEST(HexTest, LeftPadShorterInput) {
    std::vector<uint8_t> padded = leftPad({0x01, 0x02}, 4);
    EXPECT_EQ(padded, (std::vector<uint8_t>{0x00, 0x00, 0x01, 0x02}));
}

TEST(HexTest, LeftPadKeepsLastBytesWhenInputTooLong) {
    std::vector<uint8_t> truncated = leftPad({0x01, 0x02, 0x03, 0x04}, 2);
    EXPECT_EQ(truncated, (std::vector<uint8_t>{0x03, 0x04}));
}

TEST(HexTest, LeftPadExactSizeIsIdentity) {
    std::vector<uint8_t> data{0xaa, 0xbb};
    EXPECT_EQ(leftPad(data, 2), data);
}

TEST(HexTest, RightPadTo32) {
    EXPECT_EQ(rightPadTo32({}).size(), 0u);
    EXPECT_EQ(rightPadTo32({0x01}).size(), 32u);
    EXPECT_EQ(rightPadTo32(std::vector<uint8_t>(32, 0x02)).size(), 32u);
    EXPECT_EQ(rightPadTo32(std::vector<uint8_t>(33, 0x03)).size(), 64u);

    std::vector<uint8_t> padded = rightPadTo32({0x01});
    EXPECT_EQ(padded[0], 0x01);
    for (size_t i = 1; i < padded.size(); ++i) {
        EXPECT_EQ(padded[i], 0x00);
    }
}
