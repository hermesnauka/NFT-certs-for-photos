include(FetchContent)

# GoogleTest is not packaged in apt on this machine (libgtest-dev/libgmock-dev absent), so it is
# fetched at configure time instead of requiring a system install. This only touches the network
# the first time a clean build directory is configured; the result is cached under
# build/_deps and reused on subsequent builds.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(nft_certs_backend_tests ${TEST_SOURCES})
# gtest_main is intentionally not linked: tests/test_main.cpp provides main() with a _Exit-based
# shutdown that sidesteps Drogon's non-static-destruction-safe teardown (see comment there).
target_link_libraries(nft_certs_backend_tests PRIVATE nftcerts_core gtest gmock)
target_include_directories(nft_certs_backend_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(nft_certs_backend_tests)
