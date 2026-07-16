#include "db/Database.h"
#include "identity/ArtistIdentityRepository.h"
#include "identity/MockKycVerificationService.h"

#include <gtest/gtest.h>

using namespace nftcerts;

class MockKycVerificationServiceTest : public ::testing::Test {
protected:
    db::Database db{":memory:"};
    identity::ArtistIdentityRepository repository{db};
    identity::MockKycVerificationService service{repository};

    void SetUp() override { db::initSchema(db); }
};

TEST_F(MockKycVerificationServiceTest, UnknownWalletIsNotVerified) {
    EXPECT_FALSE(service.isVerified("0xunknown"));
}

TEST_F(MockKycVerificationServiceTest, VerifyAlwaysApproves) {
    identity::ArtistIdentity identity = service.verify("0xwallet", "did:key:z1", "artist@example.com");
    EXPECT_TRUE(identity.verified);
    EXPECT_EQ(identity.did, "did:key:z1");
    EXPECT_TRUE(service.isVerified("0xwallet"));
}

TEST_F(MockKycVerificationServiceTest, ReVerifyingUpdatesExistingRecord) {
    service.verify("0xwallet", "did:key:z1", "first@example.com");
    identity::ArtistIdentity updated = service.verify("0xwallet", "did:key:z2", "second@example.com");

    EXPECT_EQ(updated.did, "did:key:z2");
    EXPECT_EQ(updated.email, "second@example.com");

    auto stored = repository.findByWalletAddress("0xwallet");
    ASSERT_TRUE(stored.has_value());
    EXPECT_EQ(stored->did, "did:key:z2");
}
