#include "hdr/thread_safe_socket_queue.h"

using namespace std;

void ThreadSafeSocketQueue::push(int socket) {
    unique_lock<mutex> lock { mut };
    sockets.push(socket);
    socket_available.notify_one();
}

int ThreadSafeSocketQueue::wait_and_pop() {
    unique_lock<mutex> lock { mut };
    socket_available.wait(lock, [this] { return !sockets.empty(); });
    int value = sockets.front();
    sockets.pop();
    return value;
}