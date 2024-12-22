cmake_minimum_required(VERSION 3.10)

# Project name and version
project(JsonDiffCompare VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE ClientSources "src/*.cpp")
file(GLOB_RECURSE ClientHeaders "src/*.h")

add_executable(${PROJECT_NAME} ${ClientSources} ${ClientHeaders})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/libs/nhlomann)