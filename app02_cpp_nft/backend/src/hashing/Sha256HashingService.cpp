#include "hashing/Sha256HashingService.h"

#include <openssl/evp.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace nftcerts::hashing {

namespace {

std::string toHex(const unsigned char* digest, unsigned int len) {
    static const char* kHexChars = "0123456789abcdef";
    std::string hex;
    hex.reserve(len * 2);
    for (unsigned int i = 0; i < len; ++i) {
        hex.push_back(kHexChars[(digest[i] >> 4) & 0xF]);
        hex.push_back(kHexChars[digest[i] & 0xF]);
    }
    return hex;
}

}  // namespace

std::string Sha256HashingService::sha256Hex(const std::vector<unsigned char>& content) const {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        throw std::runtime_error("Failed to allocate SHA-256 digest context");
    }
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, content.data(), content.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, digest, &digestLen) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("SHA-256 digest computation failed");
    }
    EVP_MD_CTX_free(ctx);

    return toHex(digest, digestLen);
}

std::string Sha256HashingService::sha256Hex(const std::string& content) const {
    std::vector<unsigned char> bytes(content.begin(), content.end());
    return sha256Hex(bytes);
}

}  // namespace nftcerts::hashing
