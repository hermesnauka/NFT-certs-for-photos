#include "blockchain/Rlp.h"

namespace nftcerts::blockchain {

namespace {

Bytes encodeLength(size_t length, uint8_t offset) {
    Bytes result;
    if (length < 56) {
        result.push_back(static_cast<uint8_t>(offset + length));
        return result;
    }
    Bytes lengthBytes;
    size_t temp = length;
    while (temp > 0) {
        lengthBytes.insert(lengthBytes.begin(), static_cast<uint8_t>(temp & 0xFF));
        temp >>= 8;
    }
    result.push_back(static_cast<uint8_t>(offset + 55 + lengthBytes.size()));
    result.insert(result.end(), lengthBytes.begin(), lengthBytes.end());
    return result;
}

}  // namespace

Bytes rlpEncodeBytes(const Bytes& data) {
    if (data.size() == 1 && data[0] < 0x80) {
        return data;
    }
    Bytes prefix = encodeLength(data.size(), 0x80);
    Bytes result = prefix;
    result.insert(result.end(), data.begin(), data.end());
    return result;
}

Bytes rlpEncodeUint(uint64_t value) {
    if (value == 0) {
        return rlpEncodeBytes({});
    }
    Bytes bytes;
    uint64_t temp = value;
    while (temp > 0) {
        bytes.insert(bytes.begin(), static_cast<uint8_t>(temp & 0xFF));
        temp >>= 8;
    }
    return rlpEncodeBytes(bytes);
}

Bytes rlpEncodeList(const std::vector<Bytes>& items) {
    Bytes payload;
    for (const auto& item : items) {
        payload.insert(payload.end(), item.begin(), item.end());
    }
    Bytes prefix = encodeLength(payload.size(), 0xC0);
    Bytes result = prefix;
    result.insert(result.end(), payload.begin(), payload.end());
    return result;
}

}  // namespace nftcerts::blockchain
