#pragma once

#include "api/UploadStore.h"
#include "hashing/Sha256HashingService.h"
#include "watermark/MetadataWatermarkService.h"

// POST /api/uploads — accepts a photograph, computes its SHA-256 "digital fingerprint", embeds a
// best-effort EXIF watermark, and stores the (possibly watermarked) bytes for later use when
// creating an artwork. Mirrors app01's UploadController. Registers its route directly with
// Drogon (see api/Routes.h) rather than being a drogon::HttpController subclass, so its
// dependencies can be plain constructor-style references instead of framework-managed singletons.
namespace nftcerts::api {

void registerUploadRoutes(hashing::Sha256HashingService& hashingService,
                           watermark::MetadataWatermarkService& watermarkService, UploadStore& uploadStore);

}  // namespace nftcerts::api
