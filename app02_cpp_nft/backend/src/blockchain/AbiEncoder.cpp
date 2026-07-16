#include "blockchain/AbiEncoder.h"

#include "blockchain/Hex.h"
#include "blockchain/Keccak256.h"

#include <stdexcept>

namespace nftcerts::blockchain {

namespace {

std::vector<uint8_t> encodeAddress(const std::string& address) {
    std::vector<uint8_t> raw = hexToBytes(address);
    if (raw.size() != 20) {
        throw std::invalid_argument("Address must be 20 bytes: " + address);
    }
    return leftPad(raw, 32);
}

std::vector<uint8_t> encodeUint256(uint64_t value) {
    std::vector<uint8_t> raw;
    uint64_t temp = value;
    while (temp > 0) {
        raw.insert(raw.begin(), static_cast<uint8_t>(temp & 0xFF));
        temp >>= 8;
    }
    return leftPad(raw, 32);
}

std::vector<uint8_t> encodeBytes32(const std::string& hex) {
    std::vector<uint8_t> raw = hexToBytes(hex);
    if (raw.size() != 32) {
        throw std::invalid_argument("bytes32 value must be 32 bytes: " + hex);
    }
    return raw;
}

std::vector<uint8_t> encodeDynamicString(const std::string& value) {
    std::vector<uint8_t> lengthWord = encodeUint256(value.size());
    std::vector<uint8_t> valueBytes(value.begin(), value.end());
    std::vector<uint8_t> padded = rightPadTo32(valueBytes);
    std::vector<uint8_t> result = lengthWord;
    result.insert(result.end(), padded.begin(), padded.end());
    return result;
}

std::vector<uint8_t> functionSelector(const std::string& signature) {
    std::vector<uint8_t> digest = keccak256(std::vector<uint8_t>(signature.begin(), signature.end()));
    return std::vector<uint8_t>(digest.begin(), digest.begin() + 4);
}

}  // namespace

std::string encodeMintCertificateCall(const std::string& to, const std::string& metadataURI,
                                       const std::string& contentHashHex, const std::string& royaltyRecipient,
                                       uint64_t royaltyFeeBasisPoints) {
    std::vector<uint8_t> selector = functionSelector(kMintCertificateSignature);

    // Head: 5 words (address, offset-to-string, bytes32, address, uint256) = 5*32 = 160 bytes.
    std::vector<uint8_t> head;
    auto append = [&head](const std::vector<uint8_t>& word) { head.insert(head.end(), word.begin(), word.end()); };

    append(encodeAddress(to));
    append(encodeUint256(160));  // offset to the dynamic `metadataURI` tail, in bytes
    append(encodeBytes32(contentHashHex));
    append(encodeAddress(royaltyRecipient));
    append(encodeUint256(royaltyFeeBasisPoints));

    std::vector<uint8_t> tail = encodeDynamicString(metadataURI);

    std::vector<uint8_t> full = selector;
    full.insert(full.end(), head.begin(), head.end());
    full.insert(full.end(), tail.begin(), tail.end());

    return bytesToHex(full);
}

std::string certificateMintedEventTopic0() {
    return keccak256Hex(std::string(kCertificateMintedEventSignature));
}

std::optional<uint64_t> decodeTokenIdFromLogs(const std::vector<EventLog>& logs) {
    std::string topic0 = certificateMintedEventTopic0();
    for (const auto& log : logs) {
        if (log.topics.empty()) {
            continue;
        }
        if (log.topics[0] != topic0) {
            continue;
        }
        if (log.topics.size() < 2) {
            continue;
        }
        std::vector<uint8_t> tokenIdBytes = hexToBytes(log.topics[1]);
        uint64_t tokenId = 0;
        for (uint8_t b : tokenIdBytes) {
            tokenId = (tokenId << 8) | b;
        }
        return tokenId;
    }
    return std::nullopt;
}

}  // namespace nftcerts::blockchain
