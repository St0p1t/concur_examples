#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <vector>


int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int make_listen_socket(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port = htons(port);
    bind(fd, (sockaddr*)&a, sizeof(a));
    listen(fd, SOMAXCONN);
    set_nonblocking(fd);
    return fd;
}

int main() {
    const int PORT = 8080;
    const int MAX_EVENTS = 1024;

    int listen_fd = make_listen_socket(PORT);

    int epfd = epoll_create1(0);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    std::vector<epoll_event> events(MAX_EVENTS);
    std::cout << "Слушаем " << PORT << "\n";

    while (true) {
        int n = epoll_wait(epfd, events.data(), MAX_EVENTS, -1);

        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                while (true) {
                    int client_fd = accept(listen_fd, nullptr, nullptr);
                    if (client_fd < 0) {
                        break;
                    }
                    set_nonblocking(client_fd);
                    epoll_event cev{};
                    cev.events = EPOLLIN;
                    cev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev);
                }
            } else {
                char buf[4096];
                ssize_t r = read(fd, buf, sizeof(buf));
                if (r > 0) {
                    write(fd, buf, r);
                } else if (r == 0 || (r < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                }
            }
        }
    }
}