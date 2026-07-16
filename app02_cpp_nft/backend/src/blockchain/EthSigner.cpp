#include "blockchain/EthSigner.h"

#include "blockchain/Hex.h"
#include "blockchain/Keccak256.h"
#include "blockchain/Rlp.h"

#include <secp256k1.h>
#include <secp256k1_recovery.h>

#include <memory>
#include <stdexcept>

namespace nftcerts::blockchain {

namespace {

using ContextPtr = std::unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)>;

ContextPtr makeContext() {
    return ContextPtr(secp256k1_context_create(SECP256K1_CONTEXT_NONE), &secp256k1_context_destroy);
}

// Strips leading zero bytes for RLP's minimal-length integer encoding of r/s.
std::vector<uint8_t> stripLeadingZeros(const std::vector<uint8_t>& bytes) {
    size_t i = 0;
    while (i < bytes.size() && bytes[i] == 0) {
        ++i;
    }
    return std::vector<uint8_t>(bytes.begin() + i, bytes.end());
}

std::vector<uint8_t> buildUnsignedRlp(const LegacyTransaction& tx) {
    std::vector<Bytes> items;
    items.push_back(rlpEncodeUint(tx.nonce));
    items.push_back(rlpEncodeUint(tx.gasPriceWei));
    items.push_back(rlpEncodeUint(tx.gasLimit));
    items.push_back(rlpEncodeBytes(hexToBytes(tx.to)));
    items.push_back(rlpEncodeUint(tx.value));
    items.push_back(rlpEncodeBytes(hexToBytes(tx.data)));
    items.push_back(rlpEncodeUint(tx.chainId));
    items.push_back(rlpEncodeUint(0));
    items.push_back(rlpEncodeUint(0));
    return rlpEncodeList(items);
}

}  // namespace

std::string deriveAddress(const std::string& privateKeyHex) {
    std::vector<uint8_t> privKey = hexToBytes(privateKeyHex);
    if (privKey.size() != 32) {
        throw std::invalid_argument("Private key must be 32 bytes");
    }

    ContextPtr ctx = makeContext();
    secp256k1_pubkey pubkey;
    if (secp256k1_ec_pubkey_create(ctx.get(), &pubkey, privKey.data()) != 1) {
        throw std::runtime_error("Invalid private key: could not derive public key");
    }

    uint8_t serialized[65];
    size_t outLen = sizeof(serialized);
    secp256k1_ec_pubkey_serialize(ctx.get(), serialized, &outLen, &pubkey, SECP256K1_EC_UNCOMPRESSED);

    // serialized[0] == 0x04 prefix; hash the 64-byte X||Y, last 20 bytes are the address.
    std::vector<uint8_t> xy(serialized + 1, serialized + 65);
    std::vector<uint8_t> digest = keccak256(xy);
    std::vector<uint8_t> address(digest.end() - 20, digest.end());
    return bytesToHex(address);
}

std::string signLegacyTransaction(const LegacyTransaction& tx, const std::string& privateKeyHex,
                                   std::string* outTxHash) {
    std::vector<uint8_t> privKey = hexToBytes(privateKeyHex);
    if (privKey.size() != 32) {
        throw std::invalid_argument("Private key must be 32 bytes");
    }

    std::vector<uint8_t> unsignedRlp = buildUnsignedRlp(tx);
    std::vector<uint8_t> messageHash = keccak256(unsignedRlp);

    ContextPtr ctx = makeContext();
    secp256k1_ecdsa_recoverable_signature sig;
    if (secp256k1_ecdsa_sign_recoverable(ctx.get(), &sig, messageHash.data(), privKey.data(), nullptr, nullptr) !=
        1) {
        throw std::runtime_error("Failed to sign transaction hash");
    }

    uint8_t compact[64];
    int recid = 0;
    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx.get(), compact, &recid, &sig);

    std::vector<uint8_t> r(compact, compact + 32);
    std::vector<uint8_t> s(compact + 32, compact + 64);

    // EIP-155: v = recoveryId + chainId*2 + 35
    uint64_t v = static_cast<uint64_t>(recid) + tx.chainId * 2 + 35;

    std::vector<Bytes> items;
    items.push_back(rlpEncodeUint(tx.nonce));
    items.push_back(rlpEncodeUint(tx.gasPriceWei));
    items.push_back(rlpEncodeUint(tx.gasLimit));
    items.push_back(rlpEncodeBytes(hexToBytes(tx.to)));
    items.push_back(rlpEncodeUint(tx.value));
    items.push_back(rlpEncodeBytes(hexToBytes(tx.data)));
    items.push_back(rlpEncodeUint(v));
    items.push_back(rlpEncodeBytes(stripLeadingZeros(r)));
    items.push_back(rlpEncodeBytes(stripLeadingZeros(s)));

    std::vector<uint8_t> signedRlp = rlpEncodeList(items);

    if (outTxHash != nullptr) {
        *outTxHash = bytesToHex(keccak256(signedRlp));
    }

    return bytesToHex(signedRlp);
}

}  // namespace nftcerts::blockchain
