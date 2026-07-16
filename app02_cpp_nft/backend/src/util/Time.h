#pragma once

#include <string>

namespace nftcerts::util {

// Returns the current instant as an ISO-8601 UTC string ("...Z"), mirroring Java's Instant.now()
// serialization used for createdAt/mintedAt/verifiedAt audit timestamps.
std::string nowIso8601();

}  // namespace nftcerts::util
