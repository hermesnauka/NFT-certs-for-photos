#include "blockchain/ContractService.h"

#include "blockchain/AbiEncoder.h"
#include "blockchain/EthSigner.h"
#include "blockchain/Hex.h"
#include "blockchain/Keccak256.h"
#include "error/Exceptions.h"

#include <chrono>
#include <thread>

namespace nftcerts::blockchain {

namespace {
constexpr const char* kDuplicateHashRevertReason = "PhotoCertificate: duplicate content hash";

bool containsDuplicateHashReason(const std::string& message) {
    return message.find(kDuplicateHashRevertReason) != std::string::npos;
}
}  // namespace

ContractService::ContractService(config::Web3Properties web3Properties)
    : web3Properties_(std::move(web3Properties)), rpc_(web3Properties_.rpcUrl) {}

MintResult ContractService::mintCertificate(const std::string& to, const std::string& metadataURI,
                                             const std::string& contentHashHex, const std::string& royaltyRecipient,
                                             uint64_t royaltyFeeBasisPoints) {
    std::string minterAddress;
    try {
        minterAddress = deriveAddress(web3Properties_.minterPrivateKey);
    } catch (const std::exception& e) {
        throw error::MintingException(std::string("Minter private key is misconfigured: ") + e.what());
    }

    uint64_t chainId = rpc_.ethChainId();
    uint64_t nonce = rpc_.ethGetTransactionCount(minterAddress);

    std::string encodedFunction =
        encodeMintCertificateCall(to, metadataURI, contentHashHex, royaltyRecipient, royaltyFeeBasisPoints);

    LegacyTransaction tx;
    tx.chainId = chainId;
    tx.nonce = nonce;
    tx.gasPriceWei = kGasPriceWei;
    tx.gasLimit = kGasLimit;
    tx.to = web3Properties_.contractAddress;
    tx.value = 0;
    tx.data = encodedFunction;

    std::string signedTx = signLegacyTransaction(tx, web3Properties_.minterPrivateKey);

    std::string txHash;
    try {
        txHash = rpc_.ethSendRawTransaction(signedTx);
    } catch (const std::exception& e) {
        if (containsDuplicateHashReason(e.what())) {
            throw error::DuplicateContentHashException(contentHashHex);
        }
        throw error::MintingException(std::string("Transaction submission failed: ") + e.what());
    }

    Json::Value receipt;
    for (int attempt = 0; attempt < kPollAttempts; ++attempt) {
        receipt = rpc_.ethGetTransactionReceipt(txHash);
        if (!receipt.isNull() && receipt.isObject()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(kPollIntervalMs));
    }

    if (receipt.isNull() || !receipt.isObject()) {
        throw error::MintingException("Timed out waiting for mintCertificate transaction receipt");
    }

    std::string status = receipt.get("status", "0x0").asString();
    if (status != "0x1") {
        throw error::MintingException("mintCertificate transaction reverted (status=" + status + ")");
    }

    std::vector<EventLog> logs;
    for (const auto& logJson : receipt["logs"]) {
        EventLog log;
        for (const auto& topic : logJson["topics"]) {
            log.topics.push_back(topic.asString());
        }
        log.data = logJson.get("data", "0x").asString();
        logs.push_back(log);
    }

    std::optional<uint64_t> tokenId = decodeTokenIdFromLogs(logs);
    if (!tokenId.has_value()) {
        throw error::MintingException(
            "mintCertificate succeeded but no CertificateMinted event log was found in the receipt");
    }

    return MintResult{*tokenId, txHash, web3Properties_.contractAddress};
}

std::optional<std::string> ContractService::tokenContentHash(uint64_t tokenId) {
    // Encode: selector(4) + uint256 tokenId, left-padded to 32 bytes.
    const std::string signature = "tokenContentHash(uint256)";
    std::vector<uint8_t> data = keccak256(std::vector<uint8_t>(signature.begin(), signature.end()));
    std::vector<uint8_t> call(data.begin(), data.begin() + 4);
    std::vector<uint8_t> tokenIdBytes;
    uint64_t temp = tokenId;
    while (temp > 0) {
        tokenIdBytes.insert(tokenIdBytes.begin(), static_cast<uint8_t>(temp & 0xFF));
        temp >>= 8;
    }
    std::vector<uint8_t> padded = leftPad(tokenIdBytes, 32);
    call.insert(call.end(), padded.begin(), padded.end());

    std::string result = rpc_.ethCall(web3Properties_.contractAddress, bytesToHex(call));
    if (result.empty() || result == "0x") {
        return std::nullopt;
    }
    return result;
}

}  // namespace nftcerts::blockchain
