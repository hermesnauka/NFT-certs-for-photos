#pragma once

#include "certificate/Certificate.h"

#include <vector>

// Renders a one-page, downloadable digital certificate of authenticity for a minted NFT
// certificate, using libharu. Mirrors app01's CertificatePdfService (OpenPDF-based there).
namespace nftcerts::pdf {

class CertificatePdfService {
public:
    std::vector<unsigned char> generate(const certificate::Certificate& certificate, const std::string& etherscanUrl,
                                         const std::string& openSeaUrl, const std::string& raribleUrl) const;
};

}  // namespace nftcerts::pdf
