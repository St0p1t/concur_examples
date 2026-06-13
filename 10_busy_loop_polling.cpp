#include <vector>
#include <algorithm>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>


int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void busy_loop_server(int listen_fd) {
    std::vector<int> clients;

    while (true) {
        int client_fd = accept(listen_fd, nullptr, nullptr);
        if (client_fd >= 0) {
            set_nonblocking(client_fd);
            clients.push_back(client_fd);
        }

        char buf[1024];
        for (size_t i = 0; i < clients.size(); ) {
            int fd = clients[i];
            ssize_t n = read(fd, buf, sizeof(buf));

            if (n > 0) {
                write(fd, buf, n);
                ++i;
            } else if (n == 0) {
                close(fd);
                clients.erase(clients.begin() + i);
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    ++i;
                } else {
                    close(fd);
                    clients.erase(clients.begin() + i);
                }
            }
        }
    }
}