#pragma once

#include "api/CertificateDtoMapper.h"
#include "certificate/CertificateService.h"

// GET /api/artists/{walletAddress}/dashboard. Mirrors app01's ArtistDashboardController.
namespace nftcerts::api {

void registerArtistDashboardRoutes(certificate::CertificateService& certificateService,
                                    CertificateDtoMapper& certificateDtoMapper);

}  // namespace nftcerts::api
