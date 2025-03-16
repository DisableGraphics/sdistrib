#pragma once
#include <string>
#include <zmq.hpp>

struct Worker {
    std::string id;               // Worker ID
    zmq::socket_t socket;         // ZMQ Socket for communication
    std::chrono::steady_clock::time_point last_seen;  // Last heartbeat timestamp

    Worker(zmq::context_t& ctx, const std::string& identity)
        : id(identity), socket(ctx, zmq::socket_type::dealer), last_seen(std::chrono::steady_clock::now()) {
        socket.setsockopt(ZMQ_IDENTITY, identity.c_str(), identity.size());
    }
};