#include <common.hpp>
//  Worker using REQ socket to do LRU routing
//
void worker_thread(int id) {
    zmq::context_t context(1);
    zmq::socket_t worker(context, ZMQ_REQ);

    s_set_id(worker);
    worker.connect("tcp://localhost:" + respport);

	std::cout << "Worker " << id << " created" << std::endl;

    //  Tell backend we're ready for work
    s_send(worker, std::string("READY"));
	srand(time(NULL));
    while (1) {
        //  Read and save all frames until we get an empty frame
        //  In this example there is only 1 but it could be more
        std::string address = s_recv(worker);
		receive_empty_message(worker);
        //  Get request, send reply
        Job request = s_recv_msgp<Job>(worker);
		
        std::cout << "Worker " << id << ": " << request.id << std::endl;
		std::cout << "Worker " << id << " finished" << std::endl;

		Image response{request.id, std::vector<uint8_t>(1024*1024, 0)};

        s_sendmore(worker, address);
        s_sendmore(worker, std::string(""));
        s_send_msgp(worker, response);
    }
}

int main() {
	srand(time(nullptr));
	worker_thread(rand());
	return 0;
}