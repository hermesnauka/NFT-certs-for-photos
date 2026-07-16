#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>

// Custom main instead of gtest_main: tests that use HttpTestServer leave a Drogon app and its
// event-loop threads running, and drogon::app().quit() during static destruction segfaults inside
// StaticFileRouter::reset() (framework shutdown is not static-destruction-safe). Since every test
// has already reported its result by now, skip C++ teardown entirely: flush output so ctest sees
// the gtest report, then _Exit with the real result code.
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    std::fflush(nullptr);
    std::_Exit(result);
}
