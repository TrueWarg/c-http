// #pragma once
#include <string>
#include <vector>
#include <thread>
#include "thread_safe_socket_queue.h"

class HttpServer {
public:
    HttpServer(
        const std::string& directory, 
        const std::string& address, 
        const std::string& port, 
        const unsigned n_threads
    );
    ~HttpServer();

    void __attribute__((noreturn)) run();

private:
    void __attribute__((noreturn)) handle_clients();

    std::vector<std::thread> workers{};
    ThreadSafeSocketQueue socket_queue{};
    std::string _address;
    std::string _port;
};