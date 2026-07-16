#include "db/Database.h"

#include <filesystem>
#include <stdexcept>

namespace nftcerts::db {

Database::Database(const std::string& path) {
    if (path != ":memory:") {
        std::filesystem::path p(path);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    }
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string message = db_ != nullptr ? sqlite3_errmsg(db_) : "unknown error";
        throw std::runtime_error("Failed to open SQLite database at '" + path + "': " + message);
    }
    exec("PRAGMA foreign_keys = ON;");
}

Database::~Database() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
    }
}

void Database::exec(const std::string& sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string message = errMsg != nullptr ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        throw std::runtime_error("SQLite error executing statement: " + message);
    }
}

void initSchema(Database& db) {
    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS artwork (
            id TEXT PRIMARY KEY,
            file_id TEXT NOT NULL,
            original_filename TEXT NOT NULL,
            sha256_hash TEXT NOT NULL UNIQUE,
            title TEXT,
            description TEXT,
            medium TEXT,
            year_created INTEGER,
            royalty_percentage_bps INTEGER NOT NULL,
            artist_wallet_address TEXT NOT NULL,
            artist_did TEXT,
            image_ipfs_uri TEXT,
            metadata_ipfs_uri TEXT,
            status TEXT NOT NULL,
            created_at TEXT NOT NULL
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS certificate (
            token_id INTEGER PRIMARY KEY,
            artwork_id TEXT NOT NULL UNIQUE REFERENCES artwork(id),
            content_hash_hex TEXT NOT NULL,
            contract_address TEXT NOT NULL,
            tx_hash TEXT NOT NULL,
            owner_address TEXT NOT NULL,
            royalty_percentage_bps INTEGER NOT NULL,
            royalty_recipient TEXT NOT NULL,
            minted_at TEXT NOT NULL
        );
    )SQL");

    db.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS artist_identity (
            id TEXT PRIMARY KEY,
            wallet_address TEXT NOT NULL UNIQUE,
            did TEXT,
            email TEXT,
            verified INTEGER NOT NULL,
            verified_at TEXT
        );
    )SQL");
}

}  // namespace nftcerts::db
