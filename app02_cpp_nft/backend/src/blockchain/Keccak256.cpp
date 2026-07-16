#include "blockchain/Keccak256.h"

#include <cstring>

namespace nftcerts::blockchain {

namespace {

constexpr int kStateSize = 25;   // 5x5 64-bit lanes = 1600 bits
constexpr int kRounds = 24;
constexpr size_t kRate = 136;    // 1088 bits, for a 256-bit output (capacity = 512 bits)
constexpr uint8_t kDomainByte = 0x01;  // original Keccak padding (NOT SHA3's 0x06)

const uint64_t kRoundConstants[kRounds] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL, 0x8000000080008000ULL,
    0x000000000000808bULL, 0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
    0x000000000000008aULL, 0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
    0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL,
};

// Rotation offsets, indexed by lane position (x + 5*y).
const int kRhoOffsets[kStateSize] = {
    0, 1, 62, 28, 27, 36, 44, 6, 55, 20, 3, 10, 43, 25, 39,
    41, 45, 15, 21, 8, 18, 2, 61, 56, 14,
};

inline uint64_t rotl64(uint64_t x, int n) {
    n &= 63;
    return n == 0 ? x : (x << n) | (x >> (64 - n));
}

void keccakF1600(uint64_t state[kStateSize]) {
    for (int round = 0; round < kRounds; ++round) {
        // Theta
        uint64_t c[5];
        for (int x = 0; x < 5; ++x) {
            c[x] = state[x] ^ state[x + 5] ^ state[x + 10] ^ state[x + 15] ^ state[x + 20];
        }
        uint64_t d[5];
        for (int x = 0; x < 5; ++x) {
            d[x] = c[(x + 4) % 5] ^ rotl64(c[(x + 1) % 5], 1);
        }
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                state[x + 5 * y] ^= d[x];
            }
        }

        // Rho + Pi
        uint64_t b[kStateSize];
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                int oldIndex = x + 5 * y;
                int newX = y;
                int newY = (2 * x + 3 * y) % 5;
                int newIndex = newX + 5 * newY;
                b[newIndex] = rotl64(state[oldIndex], kRhoOffsets[oldIndex]);
            }
        }

        // Chi
        for (int y = 0; y < 5; ++y) {
            for (int x = 0; x < 5; ++x) {
                state[x + 5 * y] = b[x + 5 * y] ^ ((~b[(x + 1) % 5 + 5 * y]) & b[(x + 2) % 5 + 5 * y]);
            }
        }

        // Iota
        state[0] ^= kRoundConstants[round];
    }
}

}  // namespace

std::vector<uint8_t> keccak256(const std::vector<uint8_t>& input) {
    uint64_t state[kStateSize] = {0};

    // Pad message: pad10*1 with a 0x01 domain-separator start bit (original Keccak, not SHA3).
    std::vector<uint8_t> padded = input;
    size_t padLen = kRate - (padded.size() % kRate);
    size_t startOfPad = padded.size();
    padded.resize(padded.size() + padLen, 0);
    padded[startOfPad] |= kDomainByte;
    padded.back() |= 0x80;

    // Absorb.
    for (size_t offset = 0; offset < padded.size(); offset += kRate) {
        for (size_t i = 0; i < kRate / 8; ++i) {
            uint64_t lane = 0;
            for (int b = 0; b < 8; ++b) {
                lane |= static_cast<uint64_t>(padded[offset + i * 8 + b]) << (8 * b);
            }
            state[i] ^= lane;
        }
        keccakF1600(state);
    }

    // Squeeze 32 bytes (output size < rate, so a single block of the state suffices).
    std::vector<uint8_t> output(32);
    for (size_t i = 0; i < 32; ++i) {
        output[i] = static_cast<uint8_t>((state[i / 8] >> (8 * (i % 8))) & 0xFF);
    }
    return output;
}

namespace {
std::string toHexString(const std::vector<uint8_t>& bytes) {
    static const char* kHexChars = "0123456789abcdef";
    std::string hex = "0x";
    hex.reserve(2 + bytes.size() * 2);
    for (uint8_t b : bytes) {
        hex.push_back(kHexChars[(b >> 4) & 0xF]);
        hex.push_back(kHexChars[b & 0xF]);
    }
    return hex;
}
}  // namespace

std::string keccak256Hex(const std::vector<uint8_t>& input) {
    return toHexString(keccak256(input));
}

std::string keccak256Hex(const std::string& input) {
    std::vector<uint8_t> bytes(input.begin(), input.end());
    return keccak256Hex(bytes);
}

}  // namespace nftcerts::blockchain
