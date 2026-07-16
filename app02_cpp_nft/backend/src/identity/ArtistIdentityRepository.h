#pragma once

#include "db/Database.h"
#include "identity/ArtistIdentity.h"

#include <optional>

namespace nftcerts::identity {

class ArtistIdentityRepository {
public:
    explicit ArtistIdentityRepository(db::Database& db) : db_(db) {}

    ArtistIdentity save(const ArtistIdentity& identity);
    std::optional<ArtistIdentity> findByWalletAddress(const std::string& walletAddress);

private:
    db::Database& db_;
};

}  // namespace nftcerts::identity
