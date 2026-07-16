#pragma once

#include "blockchain/JsonRpcClient.h"
#include "blockchain/MintResult.h"
#include "config/Config.h"

#include <cstdint>
#include <optional>
#include <string>

// Manually builds and sends `mintCertificate` transactions against the deployed
// `PhotoCertificate` contract, matching the exact ABI signature defined in
// contracts/contracts/PhotoCertificate.sol. Mirrors app01's Web3j-based ContractService, but with
// hand-rolled ABI encoding/RLP/signing instead of a high-level Ethereum client library (see
// blockchain/AbiEncoder.h, Rlp.h, EthSigner.h, Keccak256.h).
namespace nftcerts::blockchain {

class ContractService {
public:
    explicit ContractService(config::Web3Properties web3Properties);

    MintResult mintCertificate(const std::string& to, const std::string& metadataURI,
                                const std::string& contentHashHex, const std::string& royaltyRecipient,
                                uint64_t royaltyFeeBasisPoints);

    // Reads a token's registered content hash via tokenContentHash(uint256) (read-only eth_call).
    std::optional<std::string> tokenContentHash(uint64_t tokenId);

private:
    config::Web3Properties web3Properties_;
    JsonRpcClient rpc_;

    static constexpr uint64_t kGasPriceWei = 20000000000ULL;  // 20 gwei, matches Web3j's DefaultGasProvider
    static constexpr uint64_t kGasLimit = 4300000ULL;          // matches Web3j's DefaultGasProvider
    static constexpr int kPollAttempts = 40;
    static constexpr int kPollIntervalMs = 1000;
};

}  // namespace nftcerts::blockchain
