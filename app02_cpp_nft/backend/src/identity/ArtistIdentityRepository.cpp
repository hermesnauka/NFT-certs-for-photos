#include "identity/ArtistIdentityRepository.h"

#include <stdexcept>

namespace nftcerts::identity {

ArtistIdentity ArtistIdentityRepository::save(const ArtistIdentity& identity) {
    static const char* kSql =
        "INSERT INTO artist_identity (id, wallet_address, did, email, verified, verified_at) "
        "VALUES (?,?,?,?,?,?) "
        "ON CONFLICT(wallet_address) DO UPDATE SET did=excluded.did, email=excluded.email, "
        "verified=excluded.verified, verified_at=excluded.verified_at";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), kSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare artist_identity save statement: ") +
                                  sqlite3_errmsg(db_.handle()));
    }

    int idx = 1;
    sqlite3_bind_text(stmt, idx++, identity.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, identity.walletAddress.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, identity.did.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, identity.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, idx++, identity.verified ? 1 : 0);
    sqlite3_bind_text(stmt, idx++, identity.verifiedAt.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(std::string("Failed to save artist identity: ") + sqlite3_errmsg(db_.handle()));
    }
    return identity;
}

std::optional<ArtistIdentity> ArtistIdentityRepository::findByWalletAddress(const std::string& walletAddress) {
    static const char* kSql =
        "SELECT id, wallet_address, did, email, verified, verified_at FROM artist_identity WHERE wallet_address = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), kSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare artist_identity query: ") +
                                  sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, walletAddress.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<ArtistIdentity> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        ArtistIdentity identity;
        identity.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        identity.walletAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) identity.did = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) identity.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        identity.verified = sqlite3_column_int(stmt, 4) != 0;
        if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) identity.verifiedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        result = identity;
    }
    sqlite3_finalize(stmt);
    return result;
}

}  // namespace nftcerts::identity
