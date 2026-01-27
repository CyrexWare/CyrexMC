cmake_minimum_required(VERSION 3.15)

include(FetchContent)

# HUNTER_ENABLED is always set if this package is included in a project using hunter (HunterGate sets it) In this case
# we will use hunter as well to stay consistent. If not the use can supply it on configure to force using hunter.
if(HUNTER_ENABLED)
  include("cmake/HunterGate.cmake")
  huntergate(URL "https://github.com/cpp-pm/hunter/archive/v0.26.6.tar.gz" SHA1
             "e70c29f878f5d5f5cdf1b9ccd628fb872e8624a8")
  message(STATUS "jwt-cpp: using hunter for dependency resolution")
endif()

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

# Set variables to control zlib's build options and avoid conflicts
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTS OFF CACHE BOOL "" FORCE)

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

SET(HUNTER_ENABLED TRUE)

FetchContent_Declare(
  wolfssl
  GIT_REPOSITORY https://github.com/wolfSSL/wolfssl
  GIT_TAG v5.8.4-stable
)
FetchContent_MakeAvailable(wolfssl)

FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp
  GIT_TAG v0.7.1
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