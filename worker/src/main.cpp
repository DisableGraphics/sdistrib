#include "worker.hpp"

int main() {
	std::srand(std::time(nullptr));
    zmq::context_t context;
	Worker worker(context);
	worker.start();
	return 0;
}