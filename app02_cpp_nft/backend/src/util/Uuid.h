#pragma once

#include <string>

namespace nftcerts::util {

// Generates a random UUIDv4 string (lowercase, hyphenated), used as the primary key for Artwork
// / ArtistIdentity records — mirrors Java's UUID.randomUUID().
std::string randomUuid();

}  // namespace nftcerts::util
