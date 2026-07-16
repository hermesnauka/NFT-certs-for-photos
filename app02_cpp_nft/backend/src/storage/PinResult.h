#pragma once

#include <string>

namespace nftcerts::storage {

struct PinResult {
    std::string cid;
    std::string uri;

    static PinResult of(const std::string& cid) { return PinResult{cid, "ipfs://" + cid}; }
};

}  // namespace nftcerts::storage
