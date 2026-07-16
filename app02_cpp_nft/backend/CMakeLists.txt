cmake_minimum_required(VERSION 3.16)
project(nft_certs_backend_cpp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Drogon REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(JSONCPP REQUIRED jsoncpp)
pkg_check_modules(SECP256K1 REQUIRED libsecp256k1)
pkg_check_modules(EXIV2 REQUIRED exiv2)

find_library(SQLITE3_LIB sqlite3 REQUIRED)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h REQUIRED)

find_library(HPDF_LIB hpdf REQUIRED)
find_path(HPDF_INCLUDE_DIR hpdf.h REQUIRED)

file(GLOB_RECURSE BACKEND_SOURCES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM BACKEND_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_library(nftcerts_core STATIC ${BACKEND_SOURCES})
target_include_directories(nftcerts_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${JSONCPP_INCLUDE_DIRS}
    ${SECP256K1_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIR}
    ${HPDF_INCLUDE_DIR}
)
target_link_libraries(nftcerts_core PUBLIC
    Drogon::Drogon
    OpenSSL::SSL
    OpenSSL::Crypto
    ${JSONCPP_LIBRARIES}
    ${SECP256K1_LIBRARIES}
    ${EXIV2_LIBRARIES}
    ${SQLITE3_LIB}
    ${HPDF_LIB}
)
target_compile_options(nftcerts_core PUBLIC ${JSONCPP_CFLAGS_OTHER} ${SECP256K1_CFLAGS_OTHER} ${EXIV2_CFLAGS_OTHER})

add_executable(nft_certs_backend src/main.cpp)
target_link_libraries(nft_certs_backend PRIVATE nftcerts_core)

option(BUILD_TESTS "Build the GTest suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
