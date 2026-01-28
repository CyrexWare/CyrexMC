cmake_minimum_required(VERSION 3.15)

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
    GIT_TAG 700207323fe306ec6154371b866360cbbf058afe
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

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/server/*.cpp"
    "${PROJECT_SOURCE_DIR}/server/*/*.cpp"
)

list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*/server/main\\.cpp$")

add_executable(CyrexMC
    server/main.cpp
    ${ENGINE_SOURCES}
)

target_compile_features(CyrexMC PRIVATE cxx_std_23)

target_include_directories(CyrexMC PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/server"
)

target_link_libraries(CyrexMC PRIVATE
    raknet
    magic_enum::magic_enum
    libdeflate_static
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
