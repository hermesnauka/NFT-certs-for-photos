#include "util/Time.h"
#include "util/Uuid.h"

#include <gtest/gtest.h>

#include <regex>
#include <set>

using namespace nftcerts::util;

TEST(UuidTest, MatchesRfc4122Version4Format) {
    std::regex pattern("^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$");
    for (int i = 0; i < 100; ++i) {
        std::string uuid = randomUuid();
        EXPECT_TRUE(std::regex_match(uuid, pattern)) << "not a v4 UUID: " << uuid;
    }
}

TEST(UuidTest, GeneratesUniqueValues) {
    std::set<std::string> seen;
    for (int i = 0; i < 1000; ++i) {
        seen.insert(randomUuid());
    }
    EXPECT_EQ(seen.size(), 1000u);
}

TEST(TimeTest, NowIso8601MatchesUtcInstantFormat) {
    std::string now = nowIso8601();
    std::regex pattern("^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}Z$");
    EXPECT_TRUE(std::regex_match(now, pattern)) << "not ISO-8601: " << now;
}
