#include <iostream>
#include <zmq.hpp>
#include <msgpack.hpp>
#include <string>
#include <vector>

// Struct representing a Job (to be received from the Worker)
struct Job {
    std::string prompt;
    int steps;
    std::string scheduler;

    MSGPACK_DEFINE(prompt, steps, scheduler);
};

// Struct representing an Image (response to be sent back to the Worker)
struct Image {
    std::vector<uint8_t> image_data;  // Binary data for the image

    MSGPACK_DEFINE(image_data);
};

int main() {
    // Initialize ZeroMQ context and socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);  // REP for receiving and responding
    socket.bind("tcp://*:5555");  // Bind to a port

    std::cout << "Server is running and waiting for a job...\n";

    while (true) {
        // Receive job from worker
        zmq::message_t request;
        auto ret = socket.recv(request);
		std::cout << ret.has_value() << std::endl;

        // Deserialize the incoming message using MessagePack
        msgpack::object_handle msg = msgpack::unpack(static_cast<char*>(request.data()), request.size());
        msgpack::object deserialized = msg.get();

        // Convert the deserialized object to a Job struct
        Job job;
        deserialized.convert(job);

        std::cout << "Received job: " << job.prompt << ", steps: " << job.steps << ", scheduler: " << job.scheduler << std::endl;

        // Simulate image generation (dummy data in this case)
        Image image;
        image.image_data = std::vector<uint8_t>(1000, 255);  // Fake image data (1KB)

        // Serialize the image response using MessagePack
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, image);

        // Send the generated image data back to the worker
        zmq::message_t reply(sbuf.size());
        memcpy(reply.data(), sbuf.data(), sbuf.size());
        socket.send(reply, zmq::send_flags::none);

        std::cout << "Sent back image data.\n";
    }

    return 0;
}
