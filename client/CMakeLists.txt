cmake_minimum_required(VERSION 3.16)
project(sdistrib-client VERSION 0.0.1 LANGUAGES CXX)

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
	src/parse.cpp
)

# Create executable
add_executable(sdistrib-client ${SOURCES})

# Link dependencies
target_link_libraries(sdistrib-client PRIVATE 
    cppzmq 
    msgpack-cxx
)

# Include directories for dependencies
target_include_directories(sdistrib-client PRIVATE 
    ${CMAKE_SOURCE_DIR}/../common
)