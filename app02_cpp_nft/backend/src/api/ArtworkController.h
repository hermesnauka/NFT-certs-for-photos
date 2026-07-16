#pragma once

#include "api/CertificateDtoMapper.h"
#include "api/UploadStore.h"
#include "certificate/CertificateService.h"

// POST /api/artworks and POST /api/artworks/{artworkId}/mint. Mirrors app01's ArtworkController.
namespace nftcerts::api {

void registerArtworkRoutes(UploadStore& uploadStore, certificate::CertificateService& certificateService,
                            CertificateDtoMapper& certificateDtoMapper);

}  // namespace nftcerts::api
