#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Manual ABI encoding for the fixed `PhotoCertificate.mintCertificate` signature and manual
// decoding of its `CertificateMinted` event log, mirroring app01's Web3j-based
// `ContractService.buildMintCertificateFunction`/`encodeMintCertificateCall` (no codegen, no
// generated contract wrapper — the encoding is explicit and independently unit-testable).
namespace nftcerts::blockchain {

// Solidity: mintCertificate(address to, string metadataURI, bytes32 contentHash,
//                            address royaltyRecipient, uint256 royaltyFeeBasisPoints)
//           returns (uint256 tokenId)
constexpr const char* kMintCertificateSignature =
    "mintCertificate(address,string,bytes32,address,uint256)";

// Solidity: event CertificateMinted(uint256 indexed tokenId, address indexed to,
//                                    bytes32 contentHash, string metadataURI)
constexpr const char* kCertificateMintedEventSignature =
    "CertificateMinted(uint256,address,bytes32,string)";

// Returns "0x" + the 4-byte function selector + ABI-encoded arguments (no "0x" gap).
// `contentHashHex` may be given with or without a "0x" prefix; must be exactly 32 bytes.
std::string encodeMintCertificateCall(const std::string& to, const std::string& metadataURI,
                                       const std::string& contentHashHex, const std::string& royaltyRecipient,
                                       uint64_t royaltyFeeBasisPoints);

// keccak256(kCertificateMintedEventSignature) as "0x"-prefixed hex — the event's topic0.
std::string certificateMintedEventTopic0();

struct EventLog {
    std::vector<std::string> topics;  // each "0x"-prefixed, 32 bytes
    std::string data;                 // "0x"-prefixed
};

// Scans `logs` for one whose topics[0] matches certificateMintedEventTopic0(), and decodes the
// indexed `tokenId` (topics[1]) from it. Returns nullopt if no matching log is present.
std::optional<uint64_t> decodeTokenIdFromLogs(const std::vector<EventLog>& logs);

}  // namespace nftcerts::blockchain
