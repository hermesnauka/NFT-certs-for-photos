#include "certificate/ArtworkRepository.h"

#include <stdexcept>

namespace nftcerts::certificate {

namespace {

Artwork readRow(sqlite3_stmt* stmt) {
    Artwork a;
    a.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    a.fileId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    a.originalFilename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    a.sha256Hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
        a.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    }
    if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
        a.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
    }
    if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
        a.medium = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    }
    if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
        a.yearCreated = sqlite3_column_int(stmt, 7);
    }
    a.royaltyPercentageBps = sqlite3_column_int(stmt, 8);
    a.artistWalletAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
    if (sqlite3_column_type(stmt, 10) != SQLITE_NULL) {
        a.artistDid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
    }
    if (sqlite3_column_type(stmt, 11) != SQLITE_NULL) {
        a.imageIpfsUri = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    }
    if (sqlite3_column_type(stmt, 12) != SQLITE_NULL) {
        a.metadataIpfsUri = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
    }
    a.status = artworkStatusFromString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13)));
    a.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
    return a;
}

constexpr const char* kSelectColumns =
    "id, file_id, original_filename, sha256_hash, title, description, medium, year_created, "
    "royalty_percentage_bps, artist_wallet_address, artist_did, image_ipfs_uri, metadata_ipfs_uri, "
    "status, created_at";

}  // namespace

Artwork ArtworkRepository::save(const Artwork& artwork) {
    static const char* kSql =
        "INSERT INTO artwork (id, file_id, original_filename, sha256_hash, title, description, medium, "
        "year_created, royalty_percentage_bps, artist_wallet_address, artist_did, image_ipfs_uri, "
        "metadata_ipfs_uri, status, created_at) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "file_id=excluded.file_id, original_filename=excluded.original_filename, "
        "sha256_hash=excluded.sha256_hash, title=excluded.title, description=excluded.description, "
        "medium=excluded.medium, year_created=excluded.year_created, "
        "royalty_percentage_bps=excluded.royalty_percentage_bps, "
        "artist_wallet_address=excluded.artist_wallet_address, artist_did=excluded.artist_did, "
        "image_ipfs_uri=excluded.image_ipfs_uri, metadata_ipfs_uri=excluded.metadata_ipfs_uri, "
        "status=excluded.status, created_at=excluded.created_at";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), kSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare artwork save statement: ") +
                                  sqlite3_errmsg(db_.handle()));
    }

    int idx = 1;
    sqlite3_bind_text(stmt, idx++, artwork.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.fileId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.originalFilename.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.sha256Hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.medium.c_str(), -1, SQLITE_TRANSIENT);
    if (artwork.yearCreated.has_value()) {
        sqlite3_bind_int(stmt, idx++, *artwork.yearCreated);
    } else {
        sqlite3_bind_null(stmt, idx++);
    }
    sqlite3_bind_int(stmt, idx++, artwork.royaltyPercentageBps);
    sqlite3_bind_text(stmt, idx++, artwork.artistWalletAddress.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.artistDid.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.imageIpfsUri.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.metadataIpfsUri.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, toString(artwork.status).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, artwork.createdAt.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(std::string("Failed to save artwork: ") + sqlite3_errmsg(db_.handle()));
    }
    return artwork;
}

std::optional<Artwork> ArtworkRepository::findById(const std::string& id) {
    std::string sql = std::string("SELECT ") + kSelectColumns + " FROM artwork WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare artwork query: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<Artwork> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = readRow(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::optional<Artwork> ArtworkRepository::findBySha256Hash(const std::string& sha256Hash) {
    std::string sql = std::string("SELECT ") + kSelectColumns + " FROM artwork WHERE sha256_hash = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare artwork query: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, sha256Hash.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<Artwork> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = readRow(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

}  // namespace nftcerts::certificate
