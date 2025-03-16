#pragma once
#include <deque>
#include <zmq.hpp>
#include <unordered_map>
#include "wstr.hpp"
#include <msg.hpp>

class Manager {
	public:
		Manager(zmq::context_t& context);
		void start();
	private:
		zmq::context_t& context;
		bool running;
		std::unordered_map<std::string, std::unique_ptr<Worker>> workers;
		std::deque<Job> job_queue;
		zmq::socket_t socket;

		void main_loop();
		void handle_register(const std::string& identity, const Message& msg);
		void handle_jobresult(const std::string& identity, const Message& msg);
		void handle_heartbeat(const std::string& identity);
		void remove_unresponsive_workers();
		void send_message(zmq::socket_t& socket, const std::string& identity, const Message& message);
};