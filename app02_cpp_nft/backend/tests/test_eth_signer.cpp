#include "blockchain/EthSigner.h"

#include <gtest/gtest.h>

using namespace nftcerts::blockchain;

namespace {
// Ground truth verified in-session: this exact private key/address/signed-tx tuple was broadcast
// to a real Hardhat in-memory chain via eth_sendRawTransaction and returned receipt status=1.
const std::string kMinterPrivateKey = "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80";
const std::string kExpectedMinterAddress = "0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266";
const std::string kCalldata =
    "0x2f0bda2c00000000000000000000000070997970c51812dc3a010c7d01b50e0d17dc79c800000000000000000000"
    "000000000000000000000000000000000000000000a0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    "aaaaaaaaaaaaaaaa00000000000000000000000070997970c51812dc3a010c7d01b50e0d17dc79c8000000000000000"
    "00000000000000000000000000000000000000000000002ee0000000000000000000000000000000000000000000"
    "000000000000000000011697066733a2f2f516d4d65746164617461000000000000000000000000000000";
const std::string kExpectedSignedRawTx =
    "0xf9014c028504a817c80083419ce0945fbdb2315678afecb367f032d93f642f64180aa380b8e42f0bda2c000000000"
    "00000000000070997970c51812dc3a010c7d01b50e0d17dc79c800000000000000000000000000000000000000000"
    "0000000000000000000a0aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000000"
    "0000000000000070997970c51812dc3a010c7d01b50e0d17dc79c800000000000000000000000000000000000000000"
    "00000000000000000002ee0000000000000000000000000000000000000000000000000000000000000011697066733"
    "a2f2f516d4d6574616461746100000000000000000000000000000082f4f5a0a0353e052d9d9f70b72d1dd2b9f72813d"
    "0747400ba1e63254d77ec9f4721bfa9a0084310bb78cfe9fba1b415fc4566c39f4185aa651ff9f69e9cedcf4cfc7ff65e";
}  // namespace

TEST(EthSignerTest, DerivesKnownHardhatMinterAddress) {
    EXPECT_EQ(deriveAddress(kMinterPrivateKey), kExpectedMinterAddress);
}

TEST(EthSignerTest, SignsLegacyTransactionMatchingRealBroadcastTransaction) {
    LegacyTransaction tx;
    tx.chainId = 31337;
    tx.nonce = 2;
    tx.gasPriceWei = 20000000000ULL;
    tx.gasLimit = 4300000;
    tx.to = "0x5fbdb2315678afecb367f032d93f642f64180aa3";
    tx.value = 0;
    tx.data = kCalldata;

    std::string txHash;
    std::string signed_ = signLegacyTransaction(tx, kMinterPrivateKey, &txHash);

    EXPECT_EQ(signed_, kExpectedSignedRawTx);
    EXPECT_EQ(txHash, "0x75121b24e3062c7648fd0a3c6404792c6422a9907f739c35e8f091a47686c7");
}
