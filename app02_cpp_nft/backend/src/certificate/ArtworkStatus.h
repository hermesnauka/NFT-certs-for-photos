#pragma once

#include <string>

namespace nftcerts::certificate {

enum class ArtworkStatus { UPLOADED, PINNED, MINTED };

inline std::string toString(ArtworkStatus status) {
    switch (status) {
        case ArtworkStatus::UPLOADED:
            return "UPLOADED";
        case ArtworkStatus::PINNED:
            return "PINNED";
        case ArtworkStatus::MINTED:
            return "MINTED";
    }
    return "UPLOADED";
}

inline ArtworkStatus artworkStatusFromString(const std::string& value) {
    if (value == "PINNED") return ArtworkStatus::PINNED;
    if (value == "MINTED") return ArtworkStatus::MINTED;
    return ArtworkStatus::UPLOADED;
}

}  // namespace nftcerts::certificate
