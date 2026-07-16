#pragma once

#include "identity/KycVerificationService.h"

// POST /api/identity/verify — mock KYC/DID verification. Mirrors app01's IdentityController.
namespace nftcerts::api {

void registerIdentityRoutes(identity::KycVerificationService& kycVerificationService);

}  // namespace nftcerts::api
