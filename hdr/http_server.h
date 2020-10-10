#include <string>
#include <thread>
#include <vector>
#include "hdr/thread_safe_socket_queue.h"

class HttpServer {
    public:
        HttpServer(
            const std::string& dir, 
            const std::string& address,
            const std::string& port,
            unsigned n_threads
        );
        ~HttpServer();
        void run();

    private:
        void handle_clients();
        std::vector<std::thread> workers;
        ThreadSafeSocketQueue socket_queue;
        std::string _address;
        std::string _port;
};