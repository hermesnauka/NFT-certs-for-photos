#pragma once

#include <string>
#include <vector>

// Embeds a small JSON payload linking a photograph to its content hash and the artist's DID
// directly into the file's EXIF metadata (Exif.Image.ImageDescription, JPEG only), providing an
// additional (non-cryptographic) provenance signal alongside the on-chain/IPFS record. Mirrors
// app01's MetadataWatermarkService (which for the same reasons documented there — a plain ASCII
// field that survives lossless rewriting reliably across library versions — uses ImageDescription
// rather than UserComment).
namespace nftcerts::watermark {

class MetadataWatermarkService {
public:
    // Embeds {"contentHash":"<hex>","artistDid":"<did>"} into the EXIF ImageDescription field of
    // a JPEG. For any other content type, the original bytes are returned unchanged (documented
    // behavior, not a bug).
    std::vector<unsigned char> watermark(const std::vector<unsigned char>& originalContent,
                                          const std::string& contentType, const std::string& contentHash,
                                          const std::string& artistDid) const;
};

}  // namespace nftcerts::watermark
