#include <common.hpp>

void client_thread(int id) {
    zmq::context_t context(1);
    zmq::socket_t client(context, ZMQ_REQ);

    s_set_id(client); // Set a printable identity
    client.connect("tcp://localhost:" + jobport);
	Job job{id, "hey hey", 32, "euler_a"};
	s_send_msgp(client, job);
	Image reply = s_recv_msgp<Image>(client);
	std::cout << "Client " << id << ": " << reply.jobid << std::endl;
}

int main(void) {
	srand(time(nullptr));
	client_thread(rand());
}