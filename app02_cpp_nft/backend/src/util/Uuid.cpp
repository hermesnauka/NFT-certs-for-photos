#include "util/Uuid.h"

#include <random>
#include <sstream>

namespace nftcerts::util {

std::string randomUuid() {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 15);
    static const char* kHex = "0123456789abcdef";

    std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    for (char& c : uuid) {
        if (c == 'x') {
            c = kHex[dist(rng)];
        } else if (c == 'y') {
            c = kHex[(dist(rng) & 0x3) | 0x8];  // variant bits per RFC 4122
        }
    }
    return uuid;
}

}  // namespace nftcerts::util
