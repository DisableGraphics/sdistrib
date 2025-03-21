cmake_minimum_required(VERSION 3.16)
project(sdistrib-manager VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(cppzmq REQUIRED)
find_package(msgpack-cxx REQUIRED)

# Include common headers
include_directories(${CMAKE_SOURCE_DIR}/common)

# Define source files
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(sdistrib-manager ${SOURCES})

# Link dependencies
target_link_libraries(sdistrib-manager PRIVATE 
    cppzmq 
    msgpack-cxx
)

# Include directories for dependencies
target_include_directories(sdistrib-manager PRIVATE 
    ${CMAKE_SOURCE_DIR}/../common
)