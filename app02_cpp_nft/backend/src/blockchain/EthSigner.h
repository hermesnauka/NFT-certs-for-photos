#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Manually builds and signs a legacy (type-0) Ethereum transaction using libsecp256k1, mirroring
// what Web3j's RawTransactionManager does internally in app01 (there, Web3j handles this; here we
// do it explicitly since there is no equivalent high-level Ethereum client library for C++).
namespace nftcerts::blockchain {

struct LegacyTransaction {
    uint64_t chainId;
    uint64_t nonce;
    uint64_t gasPriceWei;
    uint64_t gasLimit;
    std::string to;   // "0x..." 20-byte address
    uint64_t value;   // wei
    std::string data;  // "0x..." ABI-encoded call data
};

// Derives the "0x"-prefixed checksummed-free (lowercase) 20-byte address for a given 32-byte
// private key (as "0x..." hex).
std::string deriveAddress(const std::string& privateKeyHex);

// Signs `tx` with `privateKeyHex` per EIP-155 and returns the RLP-encoded signed transaction as
// "0x"-prefixed hex, ready for eth_sendRawTransaction. Also returns the resulting transaction
// hash (keccak256 of the signed RLP encoding) via `outTxHash` if non-null.
std::string signLegacyTransaction(const LegacyTransaction& tx, const std::string& privateKeyHex,
                                   std::string* outTxHash = nullptr);

}  // namespace nftcerts::blockchain
