#include "certificate/CertificateRepository.h"

#include <stdexcept>

namespace nftcerts::certificate {

namespace {

// Joins certificate with its artwork row (mirrors the Java entity's EAGER @OneToOne fetch).
constexpr const char* kJoinSelect =
    "SELECT c.token_id, c.content_hash_hex, c.contract_address, c.tx_hash, c.owner_address, "
    "c.royalty_percentage_bps, c.royalty_recipient, c.minted_at, "
    "a.id, a.file_id, a.original_filename, a.sha256_hash, a.title, a.description, a.medium, "
    "a.year_created, a.royalty_percentage_bps, a.artist_wallet_address, a.artist_did, "
    "a.image_ipfs_uri, a.metadata_ipfs_uri, a.status, a.created_at "
    "FROM certificate c JOIN artwork a ON a.id = c.artwork_id ";

Certificate readRow(sqlite3_stmt* stmt) {
    Certificate cert;
    cert.tokenId = static_cast<uint64_t>(sqlite3_column_int64(stmt, 0));
    cert.contentHashHex = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    cert.contractAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    cert.txHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    cert.ownerAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    cert.royaltyPercentageBps = sqlite3_column_int(stmt, 5);
    cert.royaltyRecipient = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    cert.mintedAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

    Artwork& a = cert.artwork;
    a.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
    a.fileId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
    a.originalFilename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
    a.sha256Hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    if (sqlite3_column_type(stmt, 12) != SQLITE_NULL) a.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
    if (sqlite3_column_type(stmt, 13) != SQLITE_NULL) a.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
    if (sqlite3_column_type(stmt, 14) != SQLITE_NULL) a.medium = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
    if (sqlite3_column_type(stmt, 15) != SQLITE_NULL) a.yearCreated = sqlite3_column_int(stmt, 15);
    a.royaltyPercentageBps = sqlite3_column_int(stmt, 16);
    a.artistWalletAddress = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 17));
    if (sqlite3_column_type(stmt, 18) != SQLITE_NULL) a.artistDid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 18));
    if (sqlite3_column_type(stmt, 19) != SQLITE_NULL) a.imageIpfsUri = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 19));
    if (sqlite3_column_type(stmt, 20) != SQLITE_NULL) a.metadataIpfsUri = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 20));
    a.status = artworkStatusFromString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 21)));
    a.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 22));

    return cert;
}

}  // namespace

Certificate CertificateRepository::save(const Certificate& certificate) {
    static const char* kSql =
        "INSERT INTO certificate (token_id, artwork_id, content_hash_hex, contract_address, tx_hash, "
        "owner_address, royalty_percentage_bps, royalty_recipient, minted_at) VALUES (?,?,?,?,?,?,?,?,?) "
        "ON CONFLICT(token_id) DO UPDATE SET artwork_id=excluded.artwork_id, "
        "content_hash_hex=excluded.content_hash_hex, contract_address=excluded.contract_address, "
        "tx_hash=excluded.tx_hash, owner_address=excluded.owner_address, "
        "royalty_percentage_bps=excluded.royalty_percentage_bps, "
        "royalty_recipient=excluded.royalty_recipient, minted_at=excluded.minted_at";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), kSql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare certificate save statement: ") +
                                  sqlite3_errmsg(db_.handle()));
    }

    int idx = 1;
    sqlite3_bind_int64(stmt, idx++, static_cast<sqlite3_int64>(certificate.tokenId));
    sqlite3_bind_text(stmt, idx++, certificate.artwork.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, certificate.contentHashHex.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, certificate.contractAddress.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, certificate.txHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, certificate.ownerAddress.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, idx++, certificate.royaltyPercentageBps);
    sqlite3_bind_text(stmt, idx++, certificate.royaltyRecipient.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, certificate.mintedAt.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(std::string("Failed to save certificate: ") + sqlite3_errmsg(db_.handle()));
    }
    return certificate;
}

std::optional<Certificate> CertificateRepository::findById(uint64_t tokenId) {
    std::string sql = std::string(kJoinSelect) + "WHERE c.token_id = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare certificate query: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(tokenId));

    std::optional<Certificate> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = readRow(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::optional<Certificate> CertificateRepository::findByContentHashHex(const std::string& contentHashHex) {
    std::string sql = std::string(kJoinSelect) + "WHERE c.content_hash_hex = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare certificate query: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, contentHashHex.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<Certificate> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = readRow(stmt);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Certificate> CertificateRepository::findByOwnerAddress(const std::string& ownerAddress) {
    std::string sql = std::string(kJoinSelect) + "WHERE c.owner_address = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Failed to prepare certificate query: ") + sqlite3_errmsg(db_.handle()));
    }
    sqlite3_bind_text(stmt, 1, ownerAddress.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<Certificate> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        results.push_back(readRow(stmt));
    }
    sqlite3_finalize(stmt);
    return results;
}

}  // namespace nftcerts::certificate
