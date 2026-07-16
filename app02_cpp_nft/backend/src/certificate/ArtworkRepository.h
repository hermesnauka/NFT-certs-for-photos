#pragma once

#include "certificate/Artwork.h"
#include "db/Database.h"

#include <optional>

namespace nftcerts::certificate {

class ArtworkRepository {
public:
    explicit ArtworkRepository(db::Database& db) : db_(db) {}

    Artwork save(const Artwork& artwork);
    std::optional<Artwork> findById(const std::string& id);
    std::optional<Artwork> findBySha256Hash(const std::string& sha256Hash);

private:
    db::Database& db_;
};

}  // namespace nftcerts::certificate
