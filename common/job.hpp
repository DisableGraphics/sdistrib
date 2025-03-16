#include <string>
#include <msgpack.hpp>

struct Job {
	int id;
    std::string prompt;
    int steps;
    std::string scheduler;
    MSGPACK_DEFINE(id, prompt, steps, scheduler);
};