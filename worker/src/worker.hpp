#pragma once
#include <zmq.hpp>
#include <msg.hpp>

class Worker {
	public:
		Worker(zmq::context_t& ctx);
	
		void start();
		void stop();
	private:
		zmq::socket_t socket;
		std::string identity = "worker_" + std::to_string(rand() % 10000);
		bool running;
		int port;
	
		void send_heartbeat();	
		void process_job(const Message& msg);
		void send_message(const Message& data);
};