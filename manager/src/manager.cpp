#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include "manager.hpp"

Manager::Manager(zmq::context_t& context) : context(context), socket(context, zmq::socket_type::router) {
	socket.bind("tcp://*:5555");
}

void Manager::send_message(zmq::socket_t& socket, const std::string& identity, const Message& message) {
    msgpack::sbuffer buffer;
    msgpack::packer<msgpack::sbuffer> packer(buffer);
    packer.pack(message);
    socket.send(zmq::message_t(buffer.data(), buffer.size()), zmq::send_flags::none);
}

void Manager::start() {
	running = true;
	for (int i = 1; i <= 6; ++i) {
        job_queue.push_back(Job{
            i,
            "Image " + std::to_string(i),
            50,
            "euler_a"
        });
    }

    std::cout << "Manager started. Waiting for workers..." << std::endl;
	while (running) {
		main_loop();
	}
}

void Manager::remove_unresponsive_workers() {
	auto now = std::chrono::steady_clock::now();
	for (auto it = workers.begin(); it != workers.end();) {
		if (now - it->second->last_seen > std::chrono::seconds(15)) {
			std::cout << "Worker " << it->first << " is unresponsive. Removing." << std::endl;
			it = workers.erase(it);
		} else {
			++it;
		}
	}
}

void Manager::handle_heartbeat(const std::string& identity) {
	std::cout << "Received heartbeat from " << identity << std::endl;
	if(workers.find(identity) != workers.end()) {
		workers[identity]->last_seen = std::chrono::steady_clock::now();
	}
}

void Manager::handle_jobresult(const std::string& identity, const Message& msg) {
	auto& image = std::get<Image>(msg.variant);
	std::cout << "Worker " << identity << " finished a job!" << std::endl;
	// Save image to file
	const std::string destination = "image_" + identity + "_" + std::to_string(image.job_id) + ".bin";
	std::ofstream image_file(destination, std::ios::binary);
	image_file.write(reinterpret_cast<const char*>(image.image_data.data()), image.image_data.size());
	std::cout << "Saved image to " << destination << std::endl;
	// Assign a new job if available
	/*if (!job_queue.empty()) {
		send_message(socket, identity, Message{ MessageType::JOB_ASSIGN, job_queue.front() });
		job_queue.pop_front();
	} else {
		std::cout << "All jobs completed!" << std::endl;
	}*/
}

void Manager::handle_register(const std::string& identity, const Message& msg) {
	auto& worker_info = std::get<WorkerInfo>(msg.variant);
	std::cout << "Worker " << identity << " registered!" << std::endl;

	// Create a worker socket and connect to it
	std::unique_ptr<Worker> worker = std::make_unique<Worker>(context, identity);
	worker->socket.connect("tcp://127.0.0.1:" + std::to_string(worker_info.port));

	// Store the worker in the map
	workers[identity] = std::move(worker);
	/*// Assign a job if available
	if (!job_queue.empty()) {
		send_message(socket, identity, Message{ MessageType::JOB_ASSIGN, job_queue.front() });
		job_queue.pop_front();
	}*/
}

void Manager::main_loop() {
	zmq::pollitem_t items[] = {
		{ socket, 0, ZMQ_POLLIN, 0 }
	};

	zmq::poll(items, 1, std::chrono::seconds(1));
	if (items[0].revents & ZMQ_POLLIN) {
		zmq::message_t identity_msg;
		zmq::message_t message;

		socket.recv(identity_msg);
		std::string identity(static_cast<char*>(identity_msg.data()), identity_msg.size());
		socket.recv(message);


		msgpack::object_handle oh = msgpack::unpack(static_cast<char*>(message.data()), message.size());
		Message msg;
		oh.get().convert(msg);
		std::cout << "Message: " << static_cast<int>(msg.type) << std::endl;
		switch (msg.type) {
			case MessageType::REGISTER_WORKER:
				handle_register(identity, msg);
				break;
			case MessageType::JOB_RESULT:
				handle_jobresult(identity, msg);
				break;
			case MessageType::HEARTBEAT:
				handle_heartbeat(identity);
				break;
			default:
				break;
		}
	}

	remove_unresponsive_workers();
}
