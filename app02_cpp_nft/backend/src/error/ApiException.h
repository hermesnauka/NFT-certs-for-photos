#pragma once

#include <drogon/HttpTypes.h>

#include <stdexcept>
#include <string>

namespace nftcerts::error {

// Base type for all domain exceptions that map to a specific HTTP status via the exception
// filter in api/ExceptionMapping.h. Concrete subclasses exist per error case so control flow
// never relies on generic/raw exceptions. Mirrors app01's ApiException.
class ApiException : public std::runtime_error {
public:
    ApiException(std::string message, drogon::HttpStatusCode status)
        : std::runtime_error(std::move(message)), status_(status) {}

    drogon::HttpStatusCode status() const { return status_; }

private:
    drogon::HttpStatusCode status_;
};

}  // namespace nftcerts::error
