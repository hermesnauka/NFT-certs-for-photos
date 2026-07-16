#include "watermark/MetadataWatermarkService.h"

#include "TinyJpeg.h"

#include <exiv2/exiv2.hpp>
#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace nftcerts;

namespace {

std::string readImageDescription(const std::vector<unsigned char>& jpegBytes) {
    auto image = Exiv2::ImageFactory::open(jpegBytes.data(), jpegBytes.size());
    image->readMetadata();
    const Exiv2::ExifData& exif = image->exifData();
    auto it = exif.findKey(Exiv2::ExifKey("Exif.Image.ImageDescription"));
    return it != exif.end() ? it->toString() : "";
}

}  // namespace

TEST(MetadataWatermarkServiceTest, EmbedsHashAndDidIntoJpegExif) {
    watermark::MetadataWatermarkService service;
    std::string contentHash(64, 'f');
    std::string did = "did:key:z6MkTestArtist";

    std::vector<unsigned char> original = testsupport::tinyJpeg();
    std::vector<unsigned char> watermarked = service.watermark(original, "image/jpeg", contentHash, did);

    EXPECT_NE(watermarked, original);

    std::string description = readImageDescription(watermarked);
    EXPECT_NE(description.find(contentHash), std::string::npos)
        << "ImageDescription does not contain the content hash: " << description;
    EXPECT_NE(description.find(did), std::string::npos)
        << "ImageDescription does not contain the artist DID: " << description;
}

TEST(MetadataWatermarkServiceTest, ContentTypeMatchIsCaseInsensitive) {
    watermark::MetadataWatermarkService service;
    std::vector<unsigned char> original = testsupport::tinyJpeg();
    std::vector<unsigned char> watermarked =
        service.watermark(original, "IMAGE/JPEG", std::string(64, 'e'), "did:key:z1");
    EXPECT_NE(watermarked, original);
}

TEST(MetadataWatermarkServiceTest, NonJpegContentTypePassesThroughUnchanged) {
    watermark::MetadataWatermarkService service;
    std::vector<unsigned char> content{'n', 'o', 't', ' ', 'j', 'p', 'e', 'g'};
    EXPECT_EQ(service.watermark(content, "image/png", std::string(64, 'a'), "did:key:z1"), content);
    EXPECT_EQ(service.watermark(content, "", std::string(64, 'a'), "did:key:z1"), content);
}

TEST(MetadataWatermarkServiceTest, CorruptJpegBytesGracefullyReturnOriginal) {
    watermark::MetadataWatermarkService service;
    std::vector<unsigned char> garbage(128, 0x5a);
    EXPECT_EQ(service.watermark(garbage, "image/jpeg", std::string(64, 'a'), "did:key:z1"), garbage);
}
