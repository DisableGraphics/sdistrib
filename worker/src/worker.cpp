#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <zmq.hpp>
#include <msgpack.hpp>
#include <job.hpp>

// -- CONFIGURATION --
const std::string MANAGER_ADDR = "tcp://localhost:5555";  // Change as needed
const int HEARTBEAT_INTERVAL = 3;  // Send a heartbeat every 3 seconds

// -- MESSAGE STRUCTURES --
enum class MessageType { REGISTER_WORKER, JOB_ASSIGN, JOB_RESULT, HEARTBEAT };

namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
	namespace adaptor {
	
	template <>
	struct convert<MessageType> {
		msgpack::object const& operator()(msgpack::object const& o, MessageType& v) const {
			int temp;
			o.convert(temp);
			v = static_cast<MessageType>(temp);
			return o;
		}
	};
	
	template <>
	struct pack<MessageType> {
		template <typename Stream>
		packer<Stream>& operator()(msgpack::packer<Stream>& o, MessageType const& v) const {
			o.pack(static_cast<int>(v));
			return o;
		}
	};
	
	template <>
	struct object_with_zone<MessageType> {
		void operator()(msgpack::object::with_zone& o, MessageType const& v) const {
			o.type = msgpack::type::POSITIVE_INTEGER;
			o.via.i64 = static_cast<int>(v);
		}
	};
	
}  // namespace adaptor
}  // MSGPACK_API_VERSION_NAMESPACE
}  // namespace msgpack

// Special case for std::monostate (Heartbeat) during serialization
namespace msgpack {
	MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
		namespace adaptor {
	
			template <>
			struct convert<std::monostate> {
				msgpack::object const& operator()(msgpack::object const& o, std::monostate& v) const {
					return o;
				}
			};
			
			template <>
			struct pack<std::monostate> {
				template <typename Stream>
				packer<Stream>& operator()(msgpack::packer<Stream>& o, std::monostate const& v) const {
					o.pack_nil();
					return o;
				}
			};
			
			template <>
			struct object_with_zone<std::monostate> {
				void operator()(msgpack::object::with_zone& o, std::monostate const& v) const {
					o.type = msgpack::type::NIL;
				}
			};
			
		}  // namespace adaptor
	}
}

struct Image {
    std::vector<uint8_t> image_data;  // Binary image data
    MSGPACK_DEFINE(image_data);
};

struct WorkerInfo {
	int port;
	int compute;
	MSGPACK_DEFINE(port, compute);
};

using MessageVariant = std::variant<
	WorkerInfo,
	Job,
	Image,
	std::monostate
>;

struct Message {
	MessageType type;
	MessageVariant variant;
	MSGPACK_DEFINE(type, variant);
};

// -- WORKER CLASS --
class Worker {
public:
    Worker(zmq::context_t& ctx) : socket(ctx, ZMQ_DEALER), running(true) {
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

    void start() {
        // Start heartbeat in a separate thread
        std::thread(&Worker::send_heartbeat, this).detach();
		
        // Register worker
        send_message({MessageType::REGISTER_WORKER, WorkerInfo{port, 10000}});

        while (running) {
            zmq::message_t message;
            if (socket.recv(&message)) {
                msgpack::object_handle oh = msgpack::unpack(static_cast<char*>(message.data()), message.size());
                MessageType msg_type;
                oh.get().convert(msg_type);

                if (msg_type == MessageType::JOB_ASSIGN) {
                    process_job(oh.get());
                }
            }
        }
    }

private:
    zmq::socket_t socket;
    std::string identity = "worker_" + std::to_string(rand() % 10000);
    bool running;
	int port;

    void send_heartbeat() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_INTERVAL));
            send_message({MessageType::HEARTBEAT});
        }
    }

    void process_job(msgpack::object obj) {
        Job job;
        obj.convert(job);
        std::cout << "Processing job: " << job.prompt << std::endl;

        // Simulate image generation (replace with real function)
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Fake image data
        Image img = {std::vector<uint8_t>(1024, 255)};
        send_message(Message{MessageType::JOB_RESULT, img});
    }

    void send_message(Message data) {
        msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> packer(buffer);
        packer.pack(data);
        socket.send(zmq::message_t(buffer.data(), buffer.size()), zmq::send_flags::none);
    }
};

int main() {
	std::srand(std::time(nullptr));
    zmq::context_t context;
    Worker worker(context);
    worker.start();
    return 0;
}
