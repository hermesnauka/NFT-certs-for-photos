#pragma once

#include <cstdint>
#include <string>

namespace nftcerts::blockchain {

struct MintResult {
    uint64_t tokenId;
    std::string txHash;
    std::string contractAddress;
};

}  // namespace nftcerts::blockchain
