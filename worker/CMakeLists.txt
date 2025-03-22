cmake_minimum_required(VERSION 3.16)
project(sdistrib-worker VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add stable-diffusion.cpp as a subdirectory
add_subdirectory(stable-diffusion.cpp)

add_compile_options(-g)

# Include common headers
include_directories(${CMAKE_SOURCE_DIR}/../common)

# Find dependencies
find_package(cppzmq REQUIRED)
find_package(msgpack-cxx REQUIRED)

# Define source files
set(SOURCES
    src/main.cpp
)

# Link stable-diffusion dependencies
# It depends on the options for hardware acceleration
if (SD_CUDA)
	set(GGML_LIBS ggml ggml-cuda ggml-base zip)
elseif(SD_VULKAN)
	set(GGML_LIBS ggml ggml-vulkan ggml-base zip)
elseif(SD_SYCL)
	set(GGML_LIBS ggml ggml-sycl ggml-base zip)
else()
	set(GGML_LIBS ggml ggml-cpu ggml-base zip)
endif()
set(GGML_LIBS ggml ggml-cpu ggml-cuda ggml-base zip)

# Create executable
add_executable(sdistrib-worker ${SOURCES})

# Link dependencies
target_link_libraries(sdistrib-worker PRIVATE 
    stable-diffusion 
    ${GGML_LIBS} 
    cppzmq 
    msgpack-cxx
)

# Include directories for dependencies
target_include_directories(sdistrib-worker PRIVATE 
    ${CMAKE_SOURCE_DIR}/common
)