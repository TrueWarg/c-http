#include "hdr/http_server.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

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
    auto server_socket = 0;
    auto servinfo = create_servinfo(port);
    for (auto p = servinfo; p != nullptr; p = p->ai_next) {
        if ((server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Server socket creation error");
            continue;
        }

        if (bind(server_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_socket);
            perror("Bind server socket error");
            continue;
        }
    }
    
    freeaddrinfo(servinfo);

    if (listen(server_socket, 10) == -1) {
        perror("Error server socket listening");
        return -1;
    }

    return server_socket
}

addrinfo* create_servinfo(const char* port)
{
    struct addrinfo hints = {};

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* servinfo;

    if (getaddrinfo(nullptr, port, &hints, &servinfo) != 0) {
        perror("Error while getting addrinfo");
        return nullptr;
    }
    return servinfo;
}