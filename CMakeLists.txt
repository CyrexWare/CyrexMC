cmake_minimum_required(VERSION 3.15)

include(FetchContent)

project(CyrexMC VERSION 1.0 LANGUAGES CXX)
enable_testing()

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
    GIT_TAG v0.9.7)
FetchContent_MakeAvailable(MagicEnum)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/cyrexmc/*.cpp")

#
# We compile CyrexMC_lib and link CyrexMC 
#
add_library(CyrexMC_lib STATIC ${SRC_FILES})
target_compile_features(CyrexMC_lib PUBLIC cxx_std_23)
target_include_directories(CyrexMC_lib PUBLIC "${PROJECT_SOURCE_DIR}/src/cyrexmc")
target_link_libraries(CyrexMC_lib PUBLIC raknet magic_enum::magic_enum)

CyrexMC_set_option(CLANG_TIDY_EXECUTABLE clang-tidy STRING "Override clang-tidy executable, requires minimum version 14")
add_custom_target(tidy
    COMMAND ${CMAKE_COMMAND} -DCLANG_TIDY_EXECUTABLE=${CLANG_TIDY_EXECUTABLE} -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR} -P ./cmake/Tidy.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} VERBATIM)

add_executable(CyrexMC "${PROJECT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(CyrexMC PRIVATE CyrexMC_lib )

# --- TESTING ---
file(GLOB_RECURSE TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*.cpp")

add_executable(RunTests ${TEST_SOURCES})
target_include_directories(RunTests PUBLIC "${PROJECT_SOURCE_DIR}/src/" "${PROJECT_SOURCE_DIR}/tests/" )
target_link_libraries(RunTests PUBLIC  GTest::gtest_main  GTest::gmock_main CyrexMC_lib)

# Auto discover TEST macros
include(GoogleTest)
gtest_discover_tests(RunTests)
