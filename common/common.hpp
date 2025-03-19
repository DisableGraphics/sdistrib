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

template <typename T>
bool s_send_msgp(zmq::socket_t& socket, const T& object, int flags = 0) {
	msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, object);
    zmq::message_t message(sbuf.size());
    memcpy (message.data(), sbuf.data(), sbuf.size());

    bool rc = socket.send (message, static_cast<zmq::send_flags>(flags)).has_value();
    return (rc);
}

template <typename T>
T s_recv_msgp (zmq::socket_t & socket, int flags = 0) {
    zmq::message_t message;
    auto recv_flags = (flags == 0) ? zmq::recv_flags::none: zmq::recv_flags::dontwait;
    (void)socket.recv(message, recv_flags);

	msgpack::unpacked unpacked;
    msgpack::unpack(unpacked, static_cast<const char*>(message.data()), message.size());
    msgpack::object obj = unpacked.get();
	T object;
	obj.convert(object);

    return object;
}


struct Job {
	int id;
	std::string prompt;
    int steps;
    std::string scheduler;
	MSGPACK_DEFINE(id, prompt, steps, scheduler);
};

struct Image {
	int jobid;
	std::vector<uint8_t> data;
	MSGPACK_DEFINE(jobid, data);
};