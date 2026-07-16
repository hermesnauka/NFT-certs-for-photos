#pragma once

#include "certificate/Certificate.h"
#include "db/Database.h"

#include <optional>
#include <vector>

namespace nftcerts::certificate {

class CertificateRepository {
public:
    explicit CertificateRepository(db::Database& db) : db_(db) {}

    Certificate save(const Certificate& certificate);
    std::optional<Certificate> findById(uint64_t tokenId);
    std::optional<Certificate> findByContentHashHex(const std::string& contentHashHex);
    std::vector<Certificate> findByOwnerAddress(const std::string& ownerAddress);

private:
    db::Database& db_;
};

}  // namespace nftcerts::certificate
