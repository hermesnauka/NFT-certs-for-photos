#pragma once

#include "blockchain/ContractService.h"
#include "certificate/ArtworkRepository.h"
#include "certificate/CertificateRepository.h"
#include "identity/KycVerificationService.h"
#include "storage/IpfsStorageService.h"

#include <optional>
#include <string>
#include <vector>

// Orchestrates the artwork -> IPFS pinning -> on-chain minting -> Certificate persistence
// pipeline. Mirrors app01's CertificateService.
namespace nftcerts::certificate {

struct StoredUpload {
    std::string originalFilename;
    std::string contentType;
    std::string sha256Hash;
    std::vector<unsigned char> content;
};

class CertificateService {
public:
    CertificateService(ArtworkRepository& artworkRepository, CertificateRepository& certificateRepository,
                        storage::IpfsStorageService& ipfsStorageService, blockchain::ContractService& contractService,
                        identity::KycVerificationService& kycVerificationService);

    Artwork createArtwork(const StoredUpload& upload, const std::string& title, const std::string& description,
                           const std::string& medium, std::optional<int> yearCreated,
                           std::optional<int> royaltyPercentageBps, const std::string& artistWalletAddress,
                           const std::string& artistDid);

    Certificate mintCertificate(const std::string& artworkId, const std::string& recipientAddress);

    Certificate getCertificate(uint64_t tokenId);
    std::vector<Certificate> getCertificatesForWallet(const std::string& walletAddress);

private:
    ArtworkRepository& artworkRepository_;
    CertificateRepository& certificateRepository_;
    storage::IpfsStorageService& ipfsStorageService_;
    blockchain::ContractService& contractService_;
    identity::KycVerificationService& kycVerificationService_;
};

}  // namespace nftcerts::certificate
