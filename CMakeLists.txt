cmake_minimum_required(VERSION 3.15)

include(FetchContent)

project(CyrexMC VERSION 1.0)

# define a macro that helps defining an option
macro(CyrexMC_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()


if(ENABLE_SANITIZERS)
    string(APPEND CMAKE_CXX_FLAGS " -fno-omit-frame-pointer -fno-sanitize-recover=all -fsanitize=undefined")
endif()

FetchContent_Declare(RakNet
    SYSTEM 
    GIT_REPOSITORY https://github.com/CyrexWare/RakNet
    GIT_TAG 700207323fe306ec6154371b866360cbbf058afe)
FetchContent_MakeAvailable(RakNet)

FetchContent_Declare(MagicEnum
    SYSTEM 
    GIT_REPOSITORY https://github.com/Neargye/magic_enum
    GIT_TAG v0.9.7)
FetchContent_MakeAvailable(MagicEnum)

FetchContent_Declare(
  libdeflate
  GIT_REPOSITORY https://github.com/ebiggers/libdeflate
  GIT_TAG v1.25
)
FetchContent_MakeAvailable(libdeflate)

SET(WOLFSSL_OPENSSLEXTRA TRUE)
FetchContent_Declare(
  wolfssl
  GIT_REPOSITORY https://github.com/wolfSSL/wolfssl
  GIT_TAG v5.8.4-stable
  OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(wolfssl)

SET(JWT_SSL_LIBRARY "wolfSSL")
FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/CyrexWare/jwt-cpp
  GIT_TAG eaebed9c588993d8dc9c791f38ac43c7220c2103
)
FetchContent_MakeAvailable(jwt-cpp)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(CyrexMC ${SRC_FILES})
target_compile_features(CyrexMC PRIVATE cxx_std_23)
target_include_directories(CyrexMC PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(CyrexMC PRIVATE raknet magic_enum::magic_enum libdeflate_static jwt-cpp wolfssl)
target_compile_definitions(CyrexMC PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

CyrexMC_set_option(CLANG_TIDY_EXECUTABLE clang-tidy STRING "Override clang-tidy executable, requires minimum version 14")
add_custom_target(tidy
    COMMAND ${CMAKE_COMMAND} -DCLANG_TIDY_EXECUTABLE=${CLANG_TIDY_EXECUTABLE} -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR} -P ./cmake/Tidy.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} VERBATIM)