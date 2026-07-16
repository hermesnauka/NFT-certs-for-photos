#include "api/I18nController.h"

#include "error/Exceptions.h"
#include "i18n/Messages.h"

#include <drogon/HttpAppFramework.h>

namespace nftcerts::api {

void registerI18nRoutes() {
    drogon::app().registerHandler(
        "/api/i18n/{1}",
        [](const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& callback,
           std::string lang) {
            const auto* messages = i18n::messagesForLanguage(lang);
            if (messages == nullptr) {
                throw error::UnsupportedLanguageException(lang);
            }

            Json::Value response;
            for (const auto& [key, value] : *messages) {
                response[key] = value;
            }
            callback(drogon::HttpResponse::newHttpJsonResponse(response));
        },
        {drogon::Get});
}

}  // namespace nftcerts::api
