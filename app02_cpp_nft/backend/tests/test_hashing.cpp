#include "hashing/Sha256HashingService.h"

#include <gtest/gtest.h>

using namespace nftcerts::hashing;

TEST(Sha256HashingServiceTest, EmptyInput) {
    Sha256HashingService service;
    // Standard published SHA-256("") test vector.
    EXPECT_EQ(service.sha256Hex(std::string("")),
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(Sha256HashingServiceTest, AbcInput) {
    Sha256HashingService service;
    // Standard published SHA-256("abc") test vector.
    EXPECT_EQ(service.sha256Hex(std::string("abc")),
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST(Sha256HashingServiceTest, DeterministicForSameInput) {
    Sha256HashingService service;
    EXPECT_EQ(service.sha256Hex(std::string("hello")), service.sha256Hex(std::string("hello")));
}

TEST(Sha256HashingServiceTest, DifferentInputsDifferentHashes) {
    Sha256HashingService service;
    EXPECT_NE(service.sha256Hex(std::string("hello")), service.sha256Hex(std::string("world")));
}
