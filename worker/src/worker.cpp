#include <iostream>
#include <thread>
#include <chrono>
#include <zmq.hpp>
#include <msgpack.hpp>
#include <msg.hpp>
#include "worker.hpp"
#include "cfg.hpp"

Worker::Worker(zmq::context_t& ctx) : socket(ctx, ZMQ_DEALER), running(true) {
	socket.setsockopt(ZMQ_IDENTITY, identity.c_str(), identity.length());

	socket.bind("tcp://*:0");
	char endpoint_buffer[256];
	size_t endpoint_size = sizeof(endpoint_buffer);
	socket.getsockopt(ZMQ_LAST_ENDPOINT, endpoint_buffer, &endpoint_size);
	std::string endpoint = endpoint_buffer;
	size_t last_colon = endpoint.rfind(":");
	port = std::stoi(endpoint.substr(last_colon + 1));
	std::cout << "Bound to " << endpoint << std::endl;
	socket.connect(MANAGER_ADDR);
}

void Worker::start() {
	// Start heartbeat in a separate thread
	std::thread(&Worker::send_heartbeat, this).detach();
	
	// Register worker
	send_message({MessageType::REGISTER_WORKER, WorkerInfo{port, 10000}});
	zmq::message_t ack;
	socket.recv(&ack);

	while (running) {
		zmq::message_t message;
		sem.acquire();
		if (socket.recv(&message)) {
			msgpack::object_handle oh = msgpack::unpack(static_cast<char*>(message.data()), message.size());
			Message msg;
			oh.get().convert(msg);

			if (msg.type == MessageType::JOB_ASSIGN) {
				process_job(msg);
			}
		}
		sem.release();
	}
}
void Worker::stop() {
	running = false;
}

void Worker::send_heartbeat() {
	static int hb = 0;
	while (running) {
		sem.acquire();
		std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_INTERVAL));
		send_message({MessageType::HEARTBEAT});
		std::cout << "Heartbeat! " << hb++ << std::endl;
		sem.release();
	}
}

void Worker::process_job(const Message& msg) {
	Job job = std::get<Job>(msg.variant);
	std::cout << "Processing job: " << job.prompt << std::endl;

	// Simulate image generation (replace with real function)
	std::this_thread::sleep_for(std::chrono::seconds(10));
	std::cout << "Finished!" << std::endl;
	// Fake image data
	Image img = {job.id, std::vector<uint8_t>(1024, 255)};
	send_message(Message{MessageType::JOB_RESULT, img});
	zmq::message_t ack;
	socket.recv(&ack);
}

void Worker::send_message(const Message& data) {
	msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> packer(buffer);
	packer.pack(data);
	std::cout << "Going to send" << std::endl;
	socket.send(zmq::message_t(buffer.data(), buffer.size()), zmq::send_flags::none);
	std::cout << "Sent" << std::endl;
}
