#pragma once
#include <msgpack.hpp>
struct WorkerInfo {
	int port;
	int compute;
	MSGPACK_DEFINE(port, compute);
};

