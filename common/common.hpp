#pragma once
#include "zhelpers.hpp"
#include <string>
#include <msgpack.hpp>

const std::string jobport = "4133";
const std::string respport = "4134";
#define CHUNK_SIZE 4096 // 4KB chunks

//  Basic request-reply client using REQ socket
//
void receive_empty_message(zmq::socket_t& sock)
{
    std::string empty = s_recv(sock);
    assert(empty.size() == 0);
}


struct Job {
	int id;
	std::string prompt;
    int steps;
    std::string scheduler;
	MSGPACK_DEFINE(prompt, steps, scheduler);
};

struct Image {
	int jobid;
	std::vector<uint8_t> data;
	MSGPACK_DEFINE(jobid, data);
};