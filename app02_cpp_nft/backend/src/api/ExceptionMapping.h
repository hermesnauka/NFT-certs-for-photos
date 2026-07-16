#pragma once

// Registers Drogon's global exception handler, translating domain exceptions (error::ApiException
// subclasses) into the uniform ErrorResponse JSON shape required by the API spec. Mirrors app01's
// GlobalExceptionHandler (@RestControllerAdvice).
namespace nftcerts::api {

void registerExceptionHandler();

}  // namespace nftcerts::api
