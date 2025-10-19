cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(TournamentMod VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

CPMAddPackage(
  NAME websocketpp
  GITHUB_REPOSITORY zaphoyd/websocketpp
  GIT_TAG 0.8.2
  DOWNLOAD_ONLY YES
)

include(cmake/FindAsio.cmake)


target_compile_definitions(${PROJECT_NAME} PUBLIC ASIO_STANDALONE WIN32_LEAN_AND_MEAN=1)
target_link_libraries(
    ${PROJECT_NAME}
     asio)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${websocketpp_SOURCE_DIR}
  ${asio_SOURCE_DIR}/asio/include
)

setup_geode_mod(${PROJECT_NAME})
