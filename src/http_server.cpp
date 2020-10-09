#include "hdr/http_server.h"
#include <iostream>
// #include <unistd.h>

using namespace std;

HttpServer::HttpServer(
    const std::string &dir,
    const std::string &address,
    const std::string &port,
    unsigned n_threads
) {
    _address = move(address);
    _port = move(port);

    if (chroot(dir.c_str()) != 0) {
        throw runtime_error("Can't chroot to specified directory");
    }
    if (daemon(0, 0) != 0){
        throw std::runtime_error("Can't daemonize process");
    }
    for (auto i = 0; i < n_threads; i++)
    {
        workers.emplace_back([this] { handle_clients(); });
    }
}

HttpServer::~HttpServer() {
    for (auto& worker: workers) {
        worker.join();
    }
}

void HttpServer::handle_clients() {
    while (true) {
        // todo handle each client
    }
}

void HttpServer::run(){
    // todo implement
}
