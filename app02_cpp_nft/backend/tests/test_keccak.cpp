#include "blockchain/Keccak256.h"

#include <gtest/gtest.h>

using namespace nftcerts::blockchain;

// Ground-truth vectors verified against ethers v6 (see app02_cpp_nft/contracts) in-session; the
// empty-string and "abc" vectors are also the standard published Keccak-256 test vectors.
TEST(Keccak256Test, EmptyInput) {
    EXPECT_EQ(keccak256Hex(std::string("")),
              "0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a47");
}

TEST(Keccak256Test, AbcInput) {
    EXPECT_EQ(keccak256Hex(std::string("abc")),
              "0x4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c4");
}

TEST(Keccak256Test, CertificateMintedEventSignature) {
    EXPECT_EQ(keccak256Hex(std::string("CertificateMinted(uint256,address,bytes32,string)")),
              "0x5628555e3ae07973161e36fc18780d8505873921d730c9ab6e6922ed4e2262");
}

TEST(Keccak256Test, OutputIs32Bytes) {
    auto digest = keccak256(std::vector<uint8_t>{'x'});
    EXPECT_EQ(digest.size(), 32u);
}
