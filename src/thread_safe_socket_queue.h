// #pragma once

#include <mutex>
#include <condition_variable>
#include <queue>

class ThreadSafeSocketQueue {
public:
    ThreadSafeSocketQueue(){}
    void push(int socket);
    int wait_and_pop();

private:
    std::mutex mut{};
    std::queue<int> sockets{};
    std::condition_variable socket_available{};
};