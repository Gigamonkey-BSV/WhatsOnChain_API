cmake_minimum_required (VERSION 3.16)

# Back compatibility for VERSION range
if (${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy (VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif ()

if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message (FATAL_ERROR "In-source builds are not allowed")
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE STRING "Modules for CMake" FORCE)

# ---------------------------------------------------------
#  Project info
# ---------------------------------------------------------
project (
    WhatsOnChain_API
    VERSION 0.0.1
    DESCRIPTION "WhatsOnChain.com API"
    LANGUAGES CXX
)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
#  Build type setup (default to Debug for single-config)
# ---------------------------------------------------------
if (NOT CMAKE_CONFIGURATION_TYPES)   # True for Make/Ninja
    if (NOT CMAKE_BUILD_TYPE)
        set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the build type." FORCE)
    endif ()
endif ()

# List of supported build types
set (_supported_build_types
    "Debug"
    "Release"
    "RelWithDebInfo"
    "MinSizeRel"
)

set (_build_type_descriptions
    "Debug          - For development. Enables debug symbols, sanitizers, and minimal optimization."
    "Release        - For distribution. Full optimization, no debug symbols, no assertions."
    "RelWithDebInfo - Optimized but retains debug info. Good for profiling and crash investigation."
    "MinSizeRel     - Focuses on shrinking the binary. Useful for constrained environments."
)

# ---------------------------------------------------------
#  User message explaining usage
# ---------------------------------------------------------
message (STATUS "")
message (STATUS "Build type selected: ${CMAKE_BUILD_TYPE}")
message (STATUS "To choose a different build type, configure with:")
message (STATUS "    cmake -DCMAKE_BUILD_TYPE=<type> ..")
message (STATUS "Available build types:")
foreach (line IN LISTS _build_type_descriptions)
    message (STATUS "  • ${line}")
endforeach ()
message (STATUS "")

# ---------------------------------------------------------
#  Compiler flags
# ---------------------------------------------------------

# Base flags for each type (C and C++)
set (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE      "-O3 -DNDEBUG")

set (CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -DNDEBUG")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

set (CMAKE_C_FLAGS_DEBUG        "-O1 -g3 -fno-omit-frame-pointer")
set (CMAKE_CXX_FLAGS_DEBUG      "-O1 -g3 -fno-omit-frame-pointer")

# ---------------------------------------------------------
#  Sanitizers (enabled only in Debug)
# ---------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message (STATUS "Enabling sanitizers for Debug build.")

    set (_san_flags "-fsanitize=address,undefined")

    add_compile_options (${_san_flags})
    add_link_options (${_san_flags})
endif ()

# ---------------------------------------------------------
#  Find Dependencies
# ---------------------------------------------------------
find_package (Boost 1.86 COMPONENTS thread chrono log_setup log REQUIRED)
# if we do not have boost
#   * linux:
#     * go to the boost website, download and extract the latest version.
#     * `./bootstrap.sh`
#     * `./b2`
#     * `sudo ./b2 install`

add_definitions ("-DHAS_BOOST")

find_package (OpenSSL REQUIRED Crypto SSL)
# if we do not have openssl
#   * on linux, `sudo apt-get install libssl-dev`

find_package (SECP256K1 REQUIRED)
# if we do not have SECP256K1
#   * on linux
#     * go to `https://github.com/bitcoin-core/secp256k1`, download the repo, and follow the instructions.
#     * you mighht need autoconf: `sudo apt install autoconf`
#     * and libtool: `sudo apt-get install libtool-bin`

find_package (PkgConfig REQUIRED)

IF (WIN32)
    pkg_check_modules (cryptopp REQUIRED IMPORTED_TARGET cryptopp)
ELSE ()
    pkg_check_modules (cryptopp REQUIRED IMPORTED_TARGET libcrypto++)
    # If we do not have crypto++, go to `https://github.com/weidai11/cryptopp` and follow the instructions.
ENDIF ()

include (FetchContent)
cmake_policy (SET CMP0135 NEW)

FetchContent_Declare (json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable (json)

FetchContent_Declare (ctre URL https://github.com/hanickadot/compile-time-regular-expressions/archive/refs/tags/v3.10.0.zip)
FetchContent_MakeAvailable (ctre)

find_package (data CONFIG REQUIRED)

find_package (net CONFIG REQUIRED)

find_package (gigamonkey CONFIG REQUIRED)

add_library (
    whatsonchain_API
    STATIC
    source/whatsonchain.cpp
)

target_link_libraries (
    whatsonchain_API
    PUBLIC
    Gigamonkey::Merkle
    Net::net
    nlohmann_json
)

target_include_directories (whatsonchain_API
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features (whatsonchain_API PUBLIC cxx_std_23)
set_target_properties (whatsonchain_API PROPERTIES CXX_EXTENSIONS OFF)

set_target_properties (whatsonchain_API PROPERTIES
    OUTPUT_NAME libwhatsonchain
    PREFIX ""
    SUFFIX ".a"
)

include (GNUInstallDirs)

# copy include directory into install destination
install (
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install the library binary (.a or .so files) to lib directory.
install (
    TARGETS whatsonchain_API nlohmann_json
    EXPORT whatsonchainTargets
    ARCHIVE
)

install (
    EXPORT whatsonchainTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/whatsonchain
    FILE whatsonchainTargets.cmake
)

include (CMakePackageConfigHelpers)

# generate the config file that includes the exports
configure_package_config_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/whatsonchainConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/whatsonchain"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file (
  "${CMAKE_CURRENT_BINARY_DIR}/whatsonchainConfigVersion.cmake"
  COMPATIBILITY AnyNewerVersion
)

install (FILES
  ${CMAKE_CURRENT_BINARY_DIR}/whatsonchainConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/whatsonchainConfigVersion.cmake
  DESTINATION lib/cmake/whatsonchain
)

set (CPACK_PACKAGE_VENDOR "Daniel Krawisz")
#set(CPACK_PACKAGE_CONTACT "email@example.com")
set (CPACK_PACKAGE_DESCRIPTION "WhatsOnChain API")
include (CPack)
