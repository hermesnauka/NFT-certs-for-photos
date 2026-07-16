#include "watermark/MetadataWatermarkService.h"

#include <exiv2/exiv2.hpp>
#include <json/json.h>

#include <iostream>

namespace nftcerts::watermark {

namespace {
constexpr const char* kJpegContentType = "image/jpeg";

std::string toLower(const std::string& s) {
    std::string result = s;
    for (char& c : result) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return result;
}

std::string buildPayloadJson(const std::string& contentHash, const std::string& artistDid) {
    Json::Value payload;
    payload["contentHash"] = contentHash;
    payload["artistDid"] = artistDid;
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    return Json::writeString(writer, payload);
}
}  // namespace

std::vector<unsigned char> MetadataWatermarkService::watermark(const std::vector<unsigned char>& originalContent,
                                                                 const std::string& contentType,
                                                                 const std::string& contentHash,
                                                                 const std::string& artistDid) const {
    if (toLower(contentType) != kJpegContentType) {
        std::cerr << "Skipping EXIF watermark embedding for unsupported content type '" << contentType
                  << "': only " << kJpegContentType << " is supported\n";
        return originalContent;
    }

    try {
        std::string payloadJson = buildPayloadJson(contentHash, artistDid);

        Exiv2::Image::UniquePtr image =
            Exiv2::ImageFactory::open(originalContent.data(), originalContent.size());
        image->readMetadata();

        Exiv2::ExifData& exifData = image->exifData();
        exifData["Exif.Image.ImageDescription"] = payloadJson;

        image->writeMetadata();

        Exiv2::BasicIo& io = image->io();
        io.seek(0, Exiv2::BasicIo::beg);
        std::vector<unsigned char> result(io.size());
        io.read(result.data(), result.size());
        return result;
    } catch (const Exiv2::Error& e) {
        std::cerr << "Failed to embed EXIF watermark, returning original file content unchanged: " << e.what()
                   << "\n";
        return originalContent;
    }
}

}  // namespace nftcerts::watermark
