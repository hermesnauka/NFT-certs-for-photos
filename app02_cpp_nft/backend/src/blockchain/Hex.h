#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

// Small hex<->bytes helpers shared by the ABI encoder, RLP/tx signer, and JSON-RPC client.
namespace nftcerts::blockchain {

inline std::string stripHexPrefix(const std::string& hex) {
    if (hex.size() >= 2 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        return hex.substr(2);
    }
    return hex;
}

inline std::vector<uint8_t> hexToBytes(const std::string& hexIn) {
    std::string hex = stripHexPrefix(hexIn);
    if (hex.size() % 2 != 0) {
        hex = "0" + hex;
    }
    std::vector<uint8_t> bytes(hex.size() / 2);
    for (size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<uint8_t>(std::stoul(hex.substr(i * 2, 2), nullptr, 16));
    }
    return bytes;
}

inline std::string bytesToHex(const std::vector<uint8_t>& bytes, bool withPrefix = true) {
    static const char* kHexChars = "0123456789abcdef";
    std::string hex = withPrefix ? "0x" : "";
    hex.reserve(hex.size() + bytes.size() * 2);
    for (uint8_t b : bytes) {
        hex.push_back(kHexChars[(b >> 4) & 0xF]);
        hex.push_back(kHexChars[b & 0xF]);
    }
    return hex;
}

// Left-pads a byte string with zeros to `size` bytes (used for ABI 32-byte words).
inline std::vector<uint8_t> leftPad(const std::vector<uint8_t>& data, size_t size) {
    if (data.size() >= size) {
        return std::vector<uint8_t>(data.end() - size, data.end());
    }
    std::vector<uint8_t> result(size - data.size(), 0);
    result.insert(result.end(), data.begin(), data.end());
    return result;
}

// Right-pads a byte string with zeros to a multiple of 32 bytes (used for ABI dynamic bytes/string).
inline std::vector<uint8_t> rightPadTo32(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> result = data;
    size_t remainder = result.size() % 32;
    if (remainder != 0) {
        result.resize(result.size() + (32 - remainder), 0);
    }
    return result;
}

}  // namespace nftcerts::blockchain
