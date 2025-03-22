#include <common.hpp>
#include <fstream>
#include "parse.hpp"

void client_fn(int id, const std::string& addr, int argc, char** argv) {
    zmq::context_t context(1);
    zmq::socket_t client(context, ZMQ_REQ);

    s_set_id(client); // Set a printable identity
    client.connect(addr);
	// Send job to the worker
	Job job;
	parse_args(argc, const_cast<const char**>(argv), job);
	s_send_msgp(client, job);
	Image reply = s_recv_msgp<Image>(client);
	std::cout << "Client " << id << ": " << reply.jobid << std::endl;
	// Check if we had an error (If we did it would be stupid to save any image)
	if(reply.error != OK) {
		std::cout << "Client " << id << ": " << "Error: " << IMG_ERROR2str(reply.error) << std::endl;
		return;
	}
	// Save the image
	std::ofstream out_file{job.output_path, std::ios::binary};
	out_file.write(reply.data.data(), reply.data.size());
}

int main(int argc, char** argv) {
	if(argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <server address> <args>" << std::endl;
		return 1;
	}
	std::string address = "tcp://" + std::string(argv[1]) + ":" + jobport;
	std::cout << "Connecting to " << address << std::endl;
	srand(time(nullptr));
	client_fn(rand(), address, argc-2, argv+2);
}