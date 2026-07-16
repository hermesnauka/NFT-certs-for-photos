#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Genuine Keccak-256 (the original Keccak submission's padding, domain byte 0x01) as used by
// Ethereum for function selectors, event topics, and address derivation. This is NOT the same as
// NIST FIPS-202 SHA3-256 (domain byte 0x06) — OpenSSL's EVP_sha3_256 would produce different
// output and must not be used here.
namespace nftcerts::blockchain {

// Returns the raw 32-byte digest.
std::vector<uint8_t> keccak256(const std::vector<uint8_t>& input);

// Returns "0x" + 64 lowercase hex chars.
std::string keccak256Hex(const std::vector<uint8_t>& input);
std::string keccak256Hex(const std::string& input);

}  // namespace nftcerts::blockchain
