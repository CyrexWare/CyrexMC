cmake_minimum_required(VERSION 3.24)

project(CyrexMC VERSION 1.0 LANGUAGES CXX)

if (MSVC)
    add_compile_options(/FS)
endif()

include(FetchContent)
enable_testing()

macro(CyrexMC_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

if(ENABLE_SANITIZERS)
    string(APPEND CMAKE_CXX_FLAGS
        " -fno-omit-frame-pointer -fno-sanitize-recover=all -fsanitize=undefined")
endif()

FetchContent_Declare(RakNet
    SYSTEM
    GIT_REPOSITORY https://github.com/CyrexWare/RakNet
    GIT_TAG 644eb2623a7ebdab28efd311124835778fe01f1e
)
FetchContent_MakeAvailable(RakNet)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(BUILD_MOCK ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(MagicEnum
    SYSTEM
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    GIT_TAG v0.9.7
)
FetchContent_MakeAvailable(MagicEnum)

FetchContent_Declare(libdeflate
    SYSTEM
    GIT_REPOSITORY https://github.com/ebiggers/libdeflate.git
    GIT_TAG v1.19
)
FetchContent_MakeAvailable(libdeflate)

FetchContent_Declare(
    stduuid
    GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
    GIT_TAG v1.2.3
)
FetchContent_MakeAvailable(stduuid)

FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG master
)
FetchContent_MakeAvailable(miniz)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
set(GLM_ENABLE_CXX_17 ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    wolfssl
    GIT_REPOSITORY https://github.com/wolfSSL/wolfssl
    GIT_TAG v5.8.4-stable
    OVERRIDE_FIND_PACKAGE
)
set(WOLFSSL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(WOLFSSL_AESGCM ON CACHE BOOL "" FORCE)
set(WOLFSSL_AESCTR ON CACHE BOOL "" FORCE)
set(WOLFSSL_AES_CTR ON CACHE BOOL "" FORCE)
set(WOLFSSL_AES_DIRECT ON CACHE BOOL "" FORCE)
set(WOLFSSL_BASE64_ENCODE ON CACHE BOOL "" FORCE)
set(WOLFSSL_OPENSSLEXTRA ON CACHE BOOL "" FORCE)
set(WOLFSSL_CRYPT_TESTS OFF CACHE BOOL "" FORCE)
set(WOLFSSL_CERTGEN ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(wolfssl)

SET(JWT_SSL_LIBRARY "wolfSSL" CACHE STRING "doc" FORCE)
FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/CyrexWare/jwt-cpp
  GIT_TAG eaebed9c588993d8dc9c791f38ac43c7220c2103
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JWT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(jwt-cpp)

# force -isystem for wolfssl
get_target_property(wolfssl_IID wolfssl INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(wolfssl PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${wolfssl_IID}")

FetchContent_Declare(
    snappy
    GIT_REPOSITORY https://github.com/google/snappy
    GIT_TAG 1.2.2
)
set(BENCHMARK_ENABLE_EXCEPTIONS ON CACHE BOOL "" FORCE)
set(SNAPPY_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(snappy)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/**.cpp"
    "${PROJECT_SOURCE_DIR}/src/**.hpp"
)

add_executable(CyrexMC
    server/main.cpp
    ${ENGINE_SOURCES}
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

target_compile_features(CyrexMC PRIVATE cxx_std_23)

target_include_directories(CyrexMC PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/server"
)

target_compile_definitions(CyrexMC PRIVATE
    WOLFSSL_AESGCM_STREAM
    WOLFSSL_AES_COUNTER
    WOLFSSL_AES_DIRECT
    OPENSSL_EXTRA
)

target_link_libraries(CyrexMC PRIVATE
    raknet
    magic_enum::magic_enum
    libdeflate_static
    wolfssl::wolfssl
    snappy
    nlohmann_json::nlohmann_json
    jwt-cpp
    glm::glm-header-only
    miniz  
    stduuid
)

CyrexMC_set_option(
    CLANG_TIDY_EXECUTABLE
    clang-tidy
    STRING
    "Override clang-tidy executable, requires minimum version 14"
)

add_custom_target(tidy
    COMMAND ${CMAKE_COMMAND}
        -DCLANG_TIDY_EXECUTABLE=${CLANG_TIDY_EXECUTABLE}
        -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}
        -P ./cmake/Tidy.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    VERBATIM
)

file(GLOB_RECURSE TEST_SOURCES
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
)

add_executable(RunTests ${TEST_SOURCES})

target_include_directories(RunTests PUBLIC "${PROJECT_SOURCE_DIR}/tests")

target_link_libraries(RunTests PUBLIC
    GTest::gtest_main
    GTest::gmock_main
)

include(GoogleTest)

gtest_discover_tests(RunTests)

