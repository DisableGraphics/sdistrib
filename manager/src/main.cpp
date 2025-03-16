#include "manager.hpp"

int main() {
	zmq::context_t context(1);
	Manager mgr{context};
	mgr.start();
	return 0;
}