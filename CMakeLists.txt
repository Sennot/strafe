cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-only for now (the hotkey polling layer is Win32); Android is a roadmap item.
set(CMAKE_OSX_ARCHITECTURES "x86_64")

project(GhostPractice VERSION 0.1.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR
        "Unable to find the Geode SDK! Install it first (https://geode-sdk.org), "
        "or just push to GitHub and let .github/workflows/build.yml build it for you.")
else()
    message(STATUS "Found Geode SDK: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

setup_geode_mod(${PROJECT_NAME})
