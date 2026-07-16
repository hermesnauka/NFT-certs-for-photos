#include "api/ExceptionMapping.h"

#include "error/ApiException.h"
#include "error/ErrorResponse.h"

#include <drogon/HttpAppFramework.h>

namespace nftcerts::api {

void registerExceptionHandler() {
    drogon::app().setExceptionHandler([](const std::exception& e, const drogon::HttpRequestPtr& req,
                                          std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
        drogon::HttpStatusCode status = drogon::k500InternalServerError;
        std::string message = e.what();

        if (const auto* apiException = dynamic_cast<const error::ApiException*>(&e)) {
            status = apiException->status();
        }

        callback(error::buildErrorResponse(status, message, req->getPath()));
    });
}

}  // namespace nftcerts::api
