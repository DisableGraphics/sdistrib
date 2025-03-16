#pragma once
#include <vector>
#include <msgpack.hpp>
#include <stdint.h>

struct Image {
	int job_id;
    std::vector<uint8_t> image_data;  // Binary image data
    MSGPACK_DEFINE(image_data);
};