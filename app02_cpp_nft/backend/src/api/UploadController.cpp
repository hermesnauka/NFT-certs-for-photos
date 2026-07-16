#include "api/UploadController.h"

#include "error/Exceptions.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/MultiPart.h>
#include <json/json.h>

#include <algorithm>
#include <set>

namespace nftcerts::api {

namespace {

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Drogon's HttpFile::getContentType() returns an internal enum rather than a raw MIME string, so
// content type is derived from the file extension instead — equivalent in practice for this
// reference implementation's fixed supported set (see docs/sdlc/05-api-design.md #1).
std::string contentTypeFromExtension(const std::string& extension) {
    std::string ext = toLower(extension);
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "webp") return "image/webp";
    return "";
}

}  // namespace

void registerUploadRoutes(hashing::Sha256HashingService& hashingService,
                           watermark::MetadataWatermarkService& watermarkService, UploadStore& uploadStore) {
    drogon::app().registerHandler(
        "/api/uploads",
        [&hashingService, &watermarkService, &uploadStore](
            const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            drogon::MultiPartParser parser;
            if (parser.parse(req) != 0 || parser.getFiles().empty()) {
                throw error::EmptyFileException();
            }

            const drogon::HttpFile& file = parser.getFiles()[0];
            if (file.fileLength() == 0) {
                throw error::EmptyFileException();
            }

            std::string contentType = contentTypeFromExtension(std::string(file.getFileExtension()));
            if (contentType.empty()) {
                throw error::UnsupportedFileTypeException(std::string(file.getFileExtension()));
            }

            std::vector<unsigned char> originalContent(file.fileData(), file.fileData() + file.fileLength());

            std::string sha256Hash = hashingService.sha256Hex(originalContent);
            // Watermark payload references the hash of the *original* content; the artist DID is
            // not yet known at upload time (supplied later on /api/artworks), so an empty marker
            // is used here — mirrors app01's UploadController.
            std::vector<unsigned char> watermarkedContent =
                watermarkService.watermark(originalContent, contentType, sha256Hash, "");

            certificate::StoredUpload stored{file.getFileName(), contentType, sha256Hash, watermarkedContent};
            std::string fileId = uploadStore.store(stored);

            Json::Value response;
            response["fileId"] = fileId;
            response["originalFilename"] = file.getFileName();
            response["sha256Hash"] = sha256Hash;
            response["sizeBytes"] = static_cast<Json::UInt64>(watermarkedContent.size());

            callback(drogon::HttpResponse::newHttpJsonResponse(response));
        },
        {drogon::Post});
}

}  // namespace nftcerts::api
