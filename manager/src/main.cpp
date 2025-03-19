//  Least-recently used (LRU) queue device
//  Clients and workers are shown here in-process
//

#include <thread>
#include <queue>
#include <common.hpp>

void client_thread(int id) {
    zmq::context_t context(1);
    zmq::socket_t client(context, ZMQ_REQ);

    s_set_id(client); // Set a printable identity
    client.connect("tcp://localhost:" + jobport);

    //  Send request, get reply
    s_send(client, std::string("HELLO " + std::to_string(id)));
    std::string reply = s_recv(client);
    std::cout << "Client: " << reply << std::endl;
    return;
}


int main(int argc, char *argv[])
{
    //  Prepare our context and sockets
    zmq::context_t context(1);
    zmq::socket_t frontend(context, ZMQ_ROUTER);
    zmq::socket_t backend(context, ZMQ_ROUTER);

    frontend.bind("tcp://*:" + jobport);
    backend.bind("tcp://*:" + respport);

    int client_nbr = 0;
    for (; client_nbr < 10; client_nbr++) {
        std::thread t(client_thread, client_nbr);
		t.detach();
    }

    //  Logic of LRU loop
    //  - Poll backend always, frontend only if 1+ worker ready
    //  - If worker replies, queue worker as ready and forward reply
    //    to client if necessary
    //  - If client requests, pop next worker and send request to it
    //
    //  A very simple queue structure with known max size
    std::queue<std::string> worker_queue;

    while(1) {
        //  Initialize poll set
        zmq::pollitem_t items[] = {
                //  Always poll for worker activity on backend
                {backend, 0, ZMQ_POLLIN, 0 },
                //  Poll front-end only if we have available workers
                {frontend, 0, ZMQ_POLLIN, 0 }
        };
        if(!worker_queue.empty())
            zmq::poll(&items[0], 2, -1);
        else
            zmq::poll(&items[0], 1, -1);

        //  Handle worker activity on backend
        if(items[0].revents & ZMQ_POLLIN) {
            //  Queue worker address for LRU routing
            worker_queue.push(s_recv(backend));
	   		receive_empty_message(backend);

            //  Third frame is READY or else a client reply address
            std::string client_addr = s_recv(backend);

            //  If client reply, send rest back to frontend
            if (client_addr.compare("READY") != 0) {
                receive_empty_message(backend);
                std::string reply = s_recv(backend);
                s_sendmore(frontend, client_addr);
                s_sendmore(frontend, std::string(""));
                s_send(frontend, reply);

                if (--client_nbr == 0)
                    break;
            }
        }
        if(items[1].revents & ZMQ_POLLIN) {
            //  Now get next client request, route to LRU worker
            //  Client request is [address][empty][request]
            std::string client_addr = s_recv(frontend);
			receive_empty_message(frontend);

            std::string request = s_recv(frontend);

            std::string worker_addr = worker_queue.front();//worker_queue [0];
            worker_queue.pop();

            s_sendmore(backend, worker_addr);
            s_sendmore(backend, std::string(""));
            s_sendmore(backend, client_addr);
            s_sendmore(backend, std::string(""));
            s_send(backend, request);
        }
    }
    return 0;
}
