#include "certificate/ArtworkRepository.h"
#include "certificate/CertificateRepository.h"
#include "db/Database.h"
#include "identity/ArtistIdentityRepository.h"

#include <gtest/gtest.h>

using namespace nftcerts;

class RepositoryTest : public ::testing::Test {
protected:
    db::Database db{":memory:"};

    void SetUp() override { db::initSchema(db); }
};

TEST_F(RepositoryTest, SavesAndFindsArtworkById) {
    certificate::ArtworkRepository repo(db);
    certificate::Artwork artwork;
    artwork.id = "artwork-1";
    artwork.fileId = "file-1";
    artwork.originalFilename = "photo.jpg";
    artwork.sha256Hash = std::string(64, 'a');
    artwork.title = "Test";
    artwork.royaltyPercentageBps = 500;
    artwork.artistWalletAddress = "0xabc";
    artwork.status = certificate::ArtworkStatus::UPLOADED;
    artwork.createdAt = "2026-01-01T00:00:00Z";

    repo.save(artwork);
    auto found = repo.findById("artwork-1");

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->title, "Test");
    EXPECT_EQ(found->status, certificate::ArtworkStatus::UPLOADED);
}

TEST_F(RepositoryTest, FindByIdReturnsNulloptWhenMissing) {
    certificate::ArtworkRepository repo(db);
    EXPECT_FALSE(repo.findById("does-not-exist").has_value());
}

TEST_F(RepositoryTest, FindsArtworkBySha256Hash) {
    certificate::ArtworkRepository repo(db);
    certificate::Artwork artwork;
    artwork.id = "artwork-2";
    artwork.fileId = "file-2";
    artwork.originalFilename = "photo2.jpg";
    artwork.sha256Hash = std::string(64, 'b');
    artwork.royaltyPercentageBps = 0;
    artwork.artistWalletAddress = "0xabc";
    artwork.status = certificate::ArtworkStatus::UPLOADED;
    artwork.createdAt = "2026-01-01T00:00:00Z";
    repo.save(artwork);

    auto found = repo.findBySha256Hash(std::string(64, 'b'));
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, "artwork-2");
}

TEST_F(RepositoryTest, SavesAndFindsCertificateWithJoinedArtwork) {
    certificate::ArtworkRepository artworkRepo(db);
    certificate::CertificateRepository certRepo(db);

    certificate::Artwork artwork;
    artwork.id = "artwork-3";
    artwork.fileId = "file-3";
    artwork.originalFilename = "photo3.jpg";
    artwork.sha256Hash = std::string(64, 'c');
    artwork.title = "Minted Piece";
    artwork.royaltyPercentageBps = 750;
    artwork.artistWalletAddress = "0xartist";
    artwork.status = certificate::ArtworkStatus::MINTED;
    artwork.createdAt = "2026-01-01T00:00:00Z";
    artworkRepo.save(artwork);

    certificate::Certificate cert;
    cert.tokenId = 42;
    cert.artwork = artwork;
    cert.contentHashHex = artwork.sha256Hash;
    cert.contractAddress = "0xcontract";
    cert.txHash = "0xtx";
    cert.ownerAddress = "0xowner";
    cert.royaltyPercentageBps = 750;
    cert.royaltyRecipient = "0xartist";
    cert.mintedAt = "2026-01-01T00:01:00Z";
    certRepo.save(cert);

    auto found = certRepo.findById(42);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->artwork.title, "Minted Piece");
    EXPECT_EQ(found->ownerAddress, "0xowner");

    auto byHash = certRepo.findByContentHashHex(artwork.sha256Hash);
    ASSERT_TRUE(byHash.has_value());
    EXPECT_EQ(byHash->tokenId, 42u);

    auto byOwner = certRepo.findByOwnerAddress("0xowner");
    ASSERT_EQ(byOwner.size(), 1u);
}

TEST_F(RepositoryTest, ArtistIdentityUpsertsByWalletAddress) {
    identity::ArtistIdentityRepository repo(db);

    identity::ArtistIdentity identity;
    identity.id = "id-1";
    identity.walletAddress = "0xwallet";
    identity.did = "did:key:z1";
    identity.verified = false;
    repo.save(identity);

    identity.verified = true;
    identity.verifiedAt = "2026-01-01T00:00:00Z";
    repo.save(identity);

    auto found = repo.findByWalletAddress("0xwallet");
    ASSERT_TRUE(found.has_value());
    EXPECT_TRUE(found->verified);
    EXPECT_EQ(found->id, "id-1");
}
