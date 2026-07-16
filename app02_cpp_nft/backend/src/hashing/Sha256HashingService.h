#pragma once

#include <string>
#include <vector>

// Computes the SHA-256 "digital fingerprint" of an uploaded artwork file. Mirrors app01's
// Sha256HashingService.
namespace nftcerts::hashing {

class Sha256HashingService {
public:
    // Returns the lowercase hex-encoded SHA-256 digest of the given bytes.
    std::string sha256Hex(const std::vector<unsigned char>& content) const;
    std::string sha256Hex(const std::string& content) const;
};

}  // namespace nftcerts::hashing
