#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Recursive Length Prefix encoding, used to serialize a legacy Ethereum transaction before
// hashing/signing. See https://ethereum.org/en/developers/docs/data-structures-and-encoding/rlp/.
namespace nftcerts::blockchain {

using Bytes = std::vector<uint8_t>;

// Encodes a single byte string per RLP rules.
Bytes rlpEncodeBytes(const Bytes& data);

// Encodes an unsigned integer (minimal big-endian bytes, no leading zeros; 0 encodes as an empty
// byte string, matching Ethereum's convention for e.g. a zero `value` or `nonce`).
Bytes rlpEncodeUint(uint64_t value);

// Encodes a list of already-RLP-encoded items into an RLP list.
Bytes rlpEncodeList(const std::vector<Bytes>& items);

}  // namespace nftcerts::blockchain
