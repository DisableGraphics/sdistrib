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
        std::string request = s_recv(worker);
		std::string number = request.substr(request.find(' ') + 1);
        std::cout << "Worker " << id << ": " << request << std::endl;
		int sleep_time = rand() % 12;
		std::cout << "Worker " << id << " sleeping for " << sleep_time << " seconds" << std::endl;
		sleep(sleep_time);
		std::cout << "Worker " << id << " finished" << std::endl;

        s_sendmore(worker, address);
        s_sendmore(worker, std::string(""));
        s_send(worker, std::string("OK " + number));
    }
    return;
}

int main() {
	srand(time(nullptr));
	worker_thread(rand());
	return 0;
}