#include "certificate/CertificateService.h"

#include "error/Exceptions.h"
#include "util/Time.h"
#include "util/Uuid.h"

#include <json/json.h>

namespace nftcerts::certificate {

CertificateService::CertificateService(ArtworkRepository& artworkRepository,
                                        CertificateRepository& certificateRepository,
                                        storage::IpfsStorageService& ipfsStorageService,
                                        blockchain::ContractService& contractService,
                                        identity::KycVerificationService& kycVerificationService)
    : artworkRepository_(artworkRepository),
      certificateRepository_(certificateRepository),
      ipfsStorageService_(ipfsStorageService),
      contractService_(contractService),
      kycVerificationService_(kycVerificationService) {}

Artwork CertificateService::createArtwork(const StoredUpload& upload, const std::string& title,
                                           const std::string& description, const std::string& medium,
                                           std::optional<int> yearCreated, std::optional<int> royaltyPercentageBps,
                                           const std::string& artistWalletAddress, const std::string& artistDid) {
    if (!royaltyPercentageBps.has_value() || *royaltyPercentageBps < 0 || *royaltyPercentageBps > 10000) {
        throw error::RoyaltyOutOfRangeException(royaltyPercentageBps.value_or(-1));
    }

    Artwork artwork;
    artwork.id = util::randomUuid();
    artwork.fileId = util::randomUuid();
    artwork.originalFilename = upload.originalFilename;
    artwork.sha256Hash = upload.sha256Hash;
    artwork.title = title;
    artwork.description = description;
    artwork.medium = medium;
    artwork.yearCreated = yearCreated;
    artwork.royaltyPercentageBps = *royaltyPercentageBps;
    artwork.artistWalletAddress = artistWalletAddress;
    artwork.artistDid = artistDid;
    artwork.status = ArtworkStatus::UPLOADED;
    artwork.createdAt = util::nowIso8601();

    storage::PinResult imagePin = ipfsStorageService_.pinFile(upload.content, upload.originalFilename, upload.contentType);
    artwork.imageIpfsUri = imagePin.uri;

    Json::Value metadata;
    metadata["title"] = artwork.title;
    metadata["description"] = artwork.description;
    metadata["medium"] = artwork.medium;
    if (artwork.yearCreated.has_value()) {
        metadata["yearCreated"] = *artwork.yearCreated;
    } else {
        metadata["yearCreated"] = Json::nullValue;
    }
    metadata["artistDid"] = artwork.artistDid;
    metadata["sha256Hash"] = artwork.sha256Hash;
    metadata["image"] = imagePin.uri;

    storage::PinResult metadataPin = ipfsStorageService_.pinJson(metadata, artwork.id + "-metadata");
    artwork.metadataIpfsUri = metadataPin.uri;

    artwork.status = ArtworkStatus::PINNED;

    return artworkRepository_.save(artwork);
}

Certificate CertificateService::mintCertificate(const std::string& artworkId, const std::string& recipientAddress) {
    std::optional<Artwork> artworkOpt = artworkRepository_.findById(artworkId);
    if (!artworkOpt.has_value()) {
        throw error::ArtworkNotFoundException(artworkId);
    }
    Artwork artwork = *artworkOpt;

    if (artwork.status != ArtworkStatus::PINNED) {
        throw error::ArtworkNotPinnedException(artworkId);
    }

    if (!kycVerificationService_.isVerified(artwork.artistWalletAddress)) {
        throw error::NotVerifiedException(artwork.artistWalletAddress);
    }

    if (certificateRepository_.findByContentHashHex(artwork.sha256Hash).has_value()) {
        throw error::DuplicateContentHashException(artwork.sha256Hash);
    }

    blockchain::MintResult mintResult = contractService_.mintCertificate(
        recipientAddress, artwork.metadataIpfsUri, artwork.sha256Hash, artwork.artistWalletAddress,
        static_cast<uint64_t>(artwork.royaltyPercentageBps));

    Certificate certificate;
    certificate.tokenId = mintResult.tokenId;
    certificate.artwork = artwork;
    certificate.contentHashHex = artwork.sha256Hash;
    certificate.contractAddress = mintResult.contractAddress;
    certificate.txHash = mintResult.txHash;
    certificate.ownerAddress = recipientAddress;
    certificate.royaltyPercentageBps = artwork.royaltyPercentageBps;
    certificate.royaltyRecipient = artwork.artistWalletAddress;
    certificate.mintedAt = util::nowIso8601();

    artwork.status = ArtworkStatus::MINTED;
    artworkRepository_.save(artwork);

    return certificateRepository_.save(certificate);
}

Certificate CertificateService::getCertificate(uint64_t tokenId) {
    auto certificate = certificateRepository_.findById(tokenId);
    if (!certificate.has_value()) {
        throw error::CertificateNotFoundException(static_cast<long long>(tokenId));
    }
    return *certificate;
}

std::vector<Certificate> CertificateService::getCertificatesForWallet(const std::string& walletAddress) {
    return certificateRepository_.findByOwnerAddress(walletAddress);
}

}  // namespace nftcerts::certificate
