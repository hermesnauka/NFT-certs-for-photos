#pragma once

#include <drogon/HttpResponse.h>

#include <string>

// Uniform error body shape returned for all mapped errors, matching app01's ErrorResponse record
// and docs/sdlc/05-api-design.md #9: {timestamp, status, error, message, path}.
namespace nftcerts::error {

drogon::HttpResponsePtr buildErrorResponse(drogon::HttpStatusCode status, const std::string& message,
                                            const std::string& path);

}  // namespace nftcerts::error
