#include "http_server.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/tcp.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

namespace {

const char RESPONSE_404[] = "HTTP/1.0 404 Not Found\r\n"
                        "Content-Type: text/html\r\n\r\n";

const char RESPONSE_200[] = "HTTP/1.0 200 OK\r\n"
                               "Content-Type: text/html\r\n\r\n";
}

HttpServer::HttpServer(
    const std::string& directory, 
    const std::string& address, 
    const std::string& port, 
    const unsigned n_threads
    )
    : _address{ std::move(address) }
    , _port{ std::move(port) }
{
    if (chroot(directory.c_str()) != 0) {
        throw std::runtime_error("Can't chroot to specified directory");
    }
    if (daemon(0, 0) == 0) {
        throw std::runtime_error("CCan't daemonize process");
    }
    for (auto i = 0u; i < n_threads; i++) {
        workers.emplace_back([this] { handle_clients(); });
    }
}
HttpServer::~HttpServer() {
    for (auto& worker: workers) {
        worker.join();
    }
}

addrinfo* create_servinfo(const char* port)
{
    struct addrinfo hints = {};
    struct addrinfo* servinfo;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(nullptr, port, &hints, &servinfo) != 0) {
        perror("Error while getting addrinfo");
        return nullptr;
    }
    return servinfo;
}

int bind_and_listen(const char* port)
{
    int server_socket = 0;
    auto servinfo = create_servinfo(port);
    for (auto p = servinfo; p != nullptr; p = p->ai_next) {
        if ((server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Server socket creation error");
            continue;
        }

        const int enable = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
            perror("Server socket reuse error");
            return 0;
        }

        if (bind(server_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_socket);
            perror("Bind server socket error");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (listen(server_socket, 10) == -1) {
        perror("Error server socket listening");
        return 0;
    }

    return server_socket;
}

std::string extract_request_path(std::string&& buf)
{
    auto newline = buf.find_first_of("\r\n");
    if (newline != std::string::npos) {
        buf = buf.substr(0, newline);
    }
    auto space = buf.find(" ");
    buf = buf.substr(space + 1);
    space = buf.find(" ");
    buf = buf.substr(0, space);

    auto question = buf.find("?");
    if (question != std::string::npos) {
        buf = buf.substr(0, question);
    }
    return buf;
}

bool send_response(int client_socket, const char *data, size_t length)
{
    if (send(client_socket, data, length, 0) == -1) {
        perror("Send response error");
        return false;
    }
    return true;
}

void handle_client(int client_socket)
{
    const int buffsize = 4096;
    char buf[buffsize];
    if (recv(client_socket, buf, buffsize, 0) == -1) {
        perror("Reciving socket data error");
    }
    auto request = extract_request_path(std::string(buf));

    if (!request.empty()) {
        struct stat st;
        auto fd = open(request.c_str(), O_RDONLY);
        if (request == "/" || fd == -1) {
            send_response(client_socket, RESPONSE_404, sizeof(RESPONSE_404));
        } else if (fstat(fd, &st) != 0) {
            perror("fstat error");
        } else {
            int enable = 1;
            if (setsockopt(client_socket, IPPROTO_TCP, TCP_CORK, &enable, sizeof(int)) == -1) {
                perror("Server socket reuse error");
            }
            send_response(client_socket, RESPONSE_200, sizeof(RESPONSE_200) - 1);
            sendfile(client_socket, fd, 0, static_cast<size_t>(st.st_size));

            enable = 0;
            if (setsockopt(client_socket, IPPROTO_TCP, TCP_CORK, &enable, sizeof(int)) == -1) {
                perror("Server socket reuse error");
            }
        }

        close(fd);
    }
    close(client_socket);
}

void HttpServer::handle_clients()
{
    while (true) {
        auto client_socket = socket_queue.wait_and_pop();
        handle_client(client_socket);
    }
}

void HttpServer::run()
{
    auto server_socket = bind_and_listen(_port.c_str());
    if (server_socket == 0) {
        throw std::runtime_error("Can't open listening socket");
    }
    while (true) {
        auto client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == -1) {
            perror("acception error");
            continue;
        }
        socket_queue.push(client_socket);
    }
}