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
    if (daemon(0, 0) != 0) {
        throw runtime_error("Can't daemonize process");
    }
    for (auto i = 0; i < n_threads; i++) {
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
        auto client_socket = socket_queue.wait_and_pop();
        handle_client(client_socket);
    }
}

void HttpServer::run() {
    auto server_socket = bind_and_listen(_port.c_str());
    if (server_socket == -1) {
        runtime_error("Can't bind listening server socket");
    }

    while (true) {
        auto client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == -1) {
            perror("Accept error");
            continue;
        }
        socket_queue.push(client_socket);
    }
}

void handle_client(int socket) {
    // todo implement
}

int bind_and_listen(const char* port) {
    // todo implement
}
