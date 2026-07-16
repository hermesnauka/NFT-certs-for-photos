#pragma once

#include <sqlite3.h>

#include <functional>
#include <memory>
#include <string>

// Thin SQLite wrapper. Plays the role of app01's H2/Spring-Data-JPA persistence layer: a
// file-based (or in-memory, for tests) SQL database with hand-written repository classes instead
// of a JPA/ORM abstraction. Not thread-safe by itself — callers serialize access (Drogon's
// synchronous controller-handler model means a single request is handled start-to-finish on one
// thread, and this codebase does not share a Database across concurrent request threads without
// external locking; see CertificateService's single shared instance for how it's used).
namespace nftcerts::db {

class Database {
public:
    // path == ":memory:" for an in-memory database (used by tests / the mock profile).
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void exec(const std::string& sql);

    sqlite3* handle() const { return db_; }

private:
    sqlite3* db_ = nullptr;
};

// Creates the artwork / certificate / artist_identity tables if they do not already exist.
void initSchema(Database& db);

}  // namespace nftcerts::db
