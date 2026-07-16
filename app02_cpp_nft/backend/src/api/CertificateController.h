#pragma once

#include "api/CertificateDtoMapper.h"
#include "certificate/CertificateService.h"
#include "pdf/CertificatePdfService.h"

// GET /api/certificates/{tokenId} and GET /api/certificates/{tokenId}/pdf. Mirrors app01's
// CertificateController.
namespace nftcerts::api {

void registerCertificateRoutes(certificate::CertificateService& certificateService,
                                CertificateDtoMapper& certificateDtoMapper,
                                pdf::CertificatePdfService& certificatePdfService);

}  // namespace nftcerts::api
