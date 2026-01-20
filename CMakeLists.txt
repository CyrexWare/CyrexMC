cmake_minimum_required(VERSION 3.15)

project(CyrexMC VERSION 1.0 LANGUAGES CXX)

# C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add executable
add_executable(CyrexMC
    cyrexmc/main.cpp
)

# Include headers
target_include_directories(CyrexMC PRIVATE cyrexmc)

# RakNet
if (EXISTS "${CMAKE_SOURCE_DIR}/RakNet/RakNet/CMakeLists.txt")
    add_subdirectory(RakNet/RakNet)
    target_link_libraries(CyrexMC PRIVATE raknet)  # just RakNet
else()
    message(FATAL_ERROR "RakNet submodule not found. Did you run git submodule update --init?")
endif()
