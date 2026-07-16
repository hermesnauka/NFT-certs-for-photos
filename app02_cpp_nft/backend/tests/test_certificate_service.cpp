#include "blockchain/ContractService.h"
#include "certificate/ArtworkRepository.h"
#include "certificate/CertificateRepository.h"
#include "certificate/CertificateService.h"
#include "db/Database.h"
#include "error/Exceptions.h"
#include "identity/ArtistIdentityRepository.h"
#include "identity/MockKycVerificationService.h"
#include "storage/LocalStubIpfsStorageService.h"

#include <gtest/gtest.h>

using namespace nftcerts;

class CertificateServiceTest : public ::testing::Test {
protected:
    db::Database db{":memory:"};
    certificate::ArtworkRepository artworkRepository{db};
    certificate::CertificateRepository certificateRepository{db};
    identity::ArtistIdentityRepository artistIdentityRepository{db};
    identity::MockKycVerificationService kycService{artistIdentityRepository};
    storage::LocalStubIpfsStorageService storageService;
    // Never actually dialed in these tests: every scenario below throws before CertificateService
    // reaches contractService_.mintCertificate(...).
    blockchain::ContractService contractService{config::Web3Properties{
        .rpcUrl = "http://localhost:1", .minterPrivateKey = std::string(64, '1'), .contractAddress = "0x" + std::string(40, '0')}};

    certificate::CertificateService service{artworkRepository, certificateRepository, storageService,
                                             contractService, kycService};

    certificate::StoredUpload makeUpload(const std::string& hash) {
        return certificate::StoredUpload{"photo.jpg", "image/jpeg", hash, {1, 2, 3, 4}};
    }
};

TEST_F(CertificateServiceTest, CreateArtworkRejectsRoyaltyOutOfRange) {
    EXPECT_THROW(service.createArtwork(makeUpload(std::string(64, 'a')), "T", "D", "M", 2024, 10001, "0xwallet", "did:key:z1"),
                 error::RoyaltyOutOfRangeException);
    EXPECT_THROW(service.createArtwork(makeUpload(std::string(64, 'a')), "T", "D", "M", 2024, -1, "0xwallet", "did:key:z1"),
                 error::RoyaltyOutOfRangeException);
}

TEST_F(CertificateServiceTest, CreateArtworkPinsAndPersistsWithPinnedStatus) {
    certificate::Artwork artwork =
        service.createArtwork(makeUpload(std::string(64, 'b')), "Title", "Desc", "Medium", 2024, 500, "0xwallet",
                               "did:key:z1");

    EXPECT_EQ(artwork.status, certificate::ArtworkStatus::PINNED);
    EXPECT_FALSE(artwork.imageIpfsUri.empty());
    EXPECT_FALSE(artwork.metadataIpfsUri.empty());
    EXPECT_TRUE(artworkRepository.findById(artwork.id).has_value());
}

TEST_F(CertificateServiceTest, MintCertificateThrowsWhenArtworkNotFound) {
    EXPECT_THROW(service.mintCertificate("does-not-exist", "0xrecipient"), error::ArtworkNotFoundException);
}

TEST_F(CertificateServiceTest, MintCertificateThrowsWhenArtworkNotPinned) {
    certificate::Artwork artwork;
    artwork.id = "unpinned-artwork";
    artwork.sha256Hash = std::string(64, 'c');
    artwork.royaltyPercentageBps = 500;
    artwork.artistWalletAddress = "0xwallet";
    artwork.status = certificate::ArtworkStatus::UPLOADED;
    artwork.createdAt = "2026-01-01T00:00:00Z";
    artworkRepository.save(artwork);

    EXPECT_THROW(service.mintCertificate("unpinned-artwork", "0xrecipient"), error::ArtworkNotPinnedException);
}

TEST_F(CertificateServiceTest, MintCertificateThrowsWhenArtistNotVerified) {
    certificate::Artwork artwork =
        service.createArtwork(makeUpload(std::string(64, 'd')), "T", "D", "M", 2024, 500, "0xunverified",
                               "did:key:z1");

    EXPECT_THROW(service.mintCertificate(artwork.id, "0xrecipient"), error::NotVerifiedException);
}

TEST_F(CertificateServiceTest, MintCertificateThrowsOnDuplicateContentHash) {
    certificate::Artwork artwork =
        service.createArtwork(makeUpload(std::string(64, 'e')), "T", "D", "M", 2024, 500, "0xverified", "did:key:z1");
    kycService.verify("0xverified", "did:key:z1", "artist@example.com");

    certificate::Certificate existing;
    existing.tokenId = 1;
    existing.artwork = artwork;
    existing.contentHashHex = artwork.sha256Hash;
    existing.contractAddress = "0xc";
    existing.txHash = "0xt";
    existing.ownerAddress = "0xowner";
    existing.royaltyPercentageBps = 500;
    existing.royaltyRecipient = "0xverified";
    existing.mintedAt = "2026-01-01T00:00:00Z";
    certificateRepository.save(existing);

    EXPECT_THROW(service.mintCertificate(artwork.id, "0xrecipient"), error::DuplicateContentHashException);
}

TEST_F(CertificateServiceTest, GetCertificateThrowsWhenNotFound) {
    EXPECT_THROW(service.getCertificate(999999), error::CertificateNotFoundException);
}

TEST_F(CertificateServiceTest, GetCertificatesForWalletReturnsEmptyForUnknownWallet) {
    EXPECT_TRUE(service.getCertificatesForWallet("0xnobody").empty());
}
