#include "blockchain/Rlp.h"

#include <gtest/gtest.h>

using namespace nftcerts::blockchain;

namespace {
std::string toHex(const Bytes& b) {
    static const char* kHex = "0123456789abcdef";
    std::string s;
    for (auto byte : b) {
        s.push_back(kHex[(byte >> 4) & 0xF]);
        s.push_back(kHex[byte & 0xF]);
    }
    return s;
}
}  // namespace

// Standard published RLP test vectors, see https://eth.wiki/fundamentals/rlp#examples.
TEST(RlpTest, EncodesEmptyString) {
    EXPECT_EQ(toHex(rlpEncodeBytes({})), "80");
}

TEST(RlpTest, EncodesSingleByteBelow0x80AsItself) {
    EXPECT_EQ(toHex(rlpEncodeBytes({0x61})), "61");  // 'a'
}

TEST(RlpTest, EncodesShortString) {
    Bytes dog = {'d', 'o', 'g'};
    EXPECT_EQ(toHex(rlpEncodeBytes(dog)), "83646f67");
}

TEST(RlpTest, EncodesZeroAsEmptyString) {
    EXPECT_EQ(toHex(rlpEncodeUint(0)), "80");
}

TEST(RlpTest, EncodesSmallUint) {
    EXPECT_EQ(toHex(rlpEncodeUint(1024)), "820400");
}

TEST(RlpTest, EncodesEmptyList) {
    EXPECT_EQ(toHex(rlpEncodeList({})), "c0");
}

TEST(RlpTest, EncodesListOfShortStrings) {
    Bytes cat = {'c', 'a', 't'};
    Bytes dog = {'d', 'o', 'g'};
    auto result = rlpEncodeList({rlpEncodeBytes(cat), rlpEncodeBytes(dog)});
    EXPECT_EQ(toHex(result), "c88363617483646f67");
}
