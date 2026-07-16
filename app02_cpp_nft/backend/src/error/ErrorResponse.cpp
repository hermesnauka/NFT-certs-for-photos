#include "error/ErrorResponse.h"

#include <json/json.h>

#include <chrono>
#include <ctime>

namespace nftcerts::error {

namespace {

std::string reasonPhrase(drogon::HttpStatusCode status) {
    switch (status) {
        case drogon::k400BadRequest:
            return "Bad Request";
        case drogon::k403Forbidden:
            return "Forbidden";
        case drogon::k404NotFound:
            return "Not Found";
        case drogon::k409Conflict:
            return "Conflict";
        case drogon::k413RequestEntityTooLarge:
            return "Payload Too Large";
        case drogon::k422UnprocessableEntity:
            return "Unprocessable Entity";
        case drogon::k502BadGateway:
            return "Bad Gateway";
        default:
            return "Error";
    }
}

std::string nowIso8601() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buf);
}

}  // namespace

drogon::HttpResponsePtr buildErrorResponse(drogon::HttpStatusCode status, const std::string& message,
                                            const std::string& path) {
    Json::Value body;
    body["timestamp"] = nowIso8601();
    body["status"] = static_cast<int>(status);
    body["error"] = reasonPhrase(status);
    body["message"] = message;
    body["path"] = path;

    auto response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(status);
    return response;
}

}  // namespace nftcerts::error
