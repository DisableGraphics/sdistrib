#include <iostream>
#include <string>
#include <unordered_map>
#include <deque>
#include <chrono>
#include <thread>
#include <zmq.hpp>
#include <msgpack.hpp>
#include <fstream>

enum class MessageType { REGISTER_WORKER, JOB_ASSIGN, JOB_RESULT, HEARTBEAT };

// --- Job Struct ---
struct Job {
    int id;
    std::string prompt;
    int steps;
    std::string scheduler;

    MSGPACK_DEFINE(id, prompt, steps, scheduler);
};

// --- Image Struct ---
struct Image {
    std::vector<uint8_t> image_data;  // Binary image data

    MSGPACK_DEFINE(image_data);
};

// --- WorkerInfo Struct ---
struct WorkerInfo {
    int port;
    int compute;

    MSGPACK_DEFINE(port, compute);
};

// --- Message Struct ---
struct Message {
    MessageType type;
    std::variant<WorkerInfo, Job, Image, std::monostate> data;

    MSGPACK_DEFINE(type, data);
};

// --- Worker Struct ---
struct Worker {
    std::string id;               // Worker ID
    zmq::socket_t socket;         // ZMQ Socket for communication
    std::chrono::steady_clock::time_point last_seen;  // Last heartbeat timestamp

    Worker(zmq::context_t& ctx, const std::string& identity)
        : socket(ctx, zmq::socket_type::dealer), id(identity), last_seen(std::chrono::steady_clock::now()) {
        socket.setsockopt(ZMQ_IDENTITY, identity.c_str(), identity.size());
    }
};

// --- Send Message Helper Function ---
void send_message(zmq::socket_t& socket, const std::string& identity, const Message& message) {
    msgpack::sbuffer buffer;
    msgpack::packer<msgpack::sbuffer> packer(buffer);
    packer.pack(message);
    socket.send(zmq::message_t(identity.data(), identity.size()), zmq::send_flags::sndmore);
    socket.send(zmq::message_t(buffer.data(), buffer.size()), zmq::send_flags::none);
}

// --- Main Manager Logic ---
int main() {
    zmq::context_t context(1);
    zmq::socket_t manager_socket(context, zmq::socket_type::router);
    manager_socket.bind("tcp://*:5555");

    std::unordered_map<std::string, Worker> workers;  // Active workers
    std::deque<Job> job_queue;  // Job queue

    // Add jobs to the queue
    for (int i = 1; i <= 6; ++i) {
        job_queue.push_back(Job{
            i,
            "Image " + std::to_string(i),
            50,
            "euler_a"
        });
    }

    std::cout << "Manager started. Waiting for workers..." << std::endl;

    while (true) {
        zmq::pollitem_t items[] = {
            { manager_socket, 0, ZMQ_POLLIN, 0 }
        };

        zmq::poll(items, 1, 1000);  // 1 sec timeout

        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t identity_msg;
            zmq::message_t message;

            manager_socket.recv(identity_msg);
            std::string identity(static_cast<char*>(identity_msg.data()), identity_msg.size());
            manager_socket.recv(message);

            msgpack::object_handle oh = msgpack::unpack(static_cast<char*>(message.data()), message.size());
            Message msg;
            oh.get().convert(msg);

            switch (msg.type) {
                case MessageType::REGISTER_WORKER: {
                    auto& worker_info = std::get<WorkerInfo>(msg.data);
                    std::cout << "Worker " << identity << " registered!" << std::endl;

                    // Create a worker socket and connect to it
                    Worker worker(context, identity);
                    worker.socket.connect("tcp://127.0.0.1:" + std::to_string(worker_info.port));

                    // Store the worker in the map
                    workers[identity] = std::move(worker);

                    // Assign a job if available
                    if (!job_queue.empty()) {
                        send_message(manager_socket, identity, Message{ MessageType::JOB_ASSIGN, job_queue.front() });
                        job_queue.pop_front();
                    }
                    break;
                }
                case MessageType::JOB_RESULT: {
                    auto& image = std::get<Image>(msg.data);
                    std::cout << "Worker " << identity << " finished a job!" << std::endl;
                    // Save image to file
                    std::ofstream image_file("image_" + identity + ".bin", std::ios::binary);
                    image_file.write(reinterpret_cast<char*>(image.image_data.data()), image.image_data.size());
                    std::cout << "Saved image to image_" << identity << ".bin" << std::endl;

                    // Assign a new job if available
                    if (!job_queue.empty()) {
                        send_message(manager_socket, identity, Message{ MessageType::JOB_ASSIGN, job_queue.front() });
                        job_queue.pop_front();
                    } else {
                        std::cout << "All jobs completed!" << std::endl;
                    }
                    break;
                }
                case MessageType::HEARTBEAT: {
                    workers[identity].last_seen = std::chrono::steady_clock::now();
                    break;
                }
                default:
                    break;
            }
        }

        // Remove unresponsive workers
        auto now = std::chrono::steady_clock::now();
        for (auto it = workers.begin(); it != workers.end();) {
            if (now - it->second.last_seen > std::chrono::seconds(5)) {
                std::cout << "Worker " << it->first << " is unresponsive. Removing." << std::endl;
                it = workers.erase(it);
            } else {
                ++it;
            }
        }
    }

    return 0;
}
