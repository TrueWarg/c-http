#include <string>
#include <thread>
#include <vector>

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
        std::vector<std::thread> workers();
        std::string address;
        std::string port;
};