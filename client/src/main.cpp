#include <common.hpp>
#include <fstream>
#include "parse.hpp"

void client_thread(int id, int argc, char** argv) {
    zmq::context_t context(1);
    zmq::socket_t client(context, ZMQ_REQ);

    s_set_id(client); // Set a printable identity
    client.connect("tcp://localhost:" + jobport);
	Job job;
	parse_args(argc, const_cast<const char**>(argv), job);
	s_send_msgp(client, job);
	Image reply = s_recv_msgp<Image>(client);
	std::cout << "Client " << id << ": " << reply.jobid << std::endl;
	std::ofstream out_file{job.output_path, std::ios::binary};
	out_file.write(reply.data.data(), reply.data.size());
}

int main(int argc, char** argv) {
	srand(time(nullptr));
	client_thread(rand(), argc, argv);
}