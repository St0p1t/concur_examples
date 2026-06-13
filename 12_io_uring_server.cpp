#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <iostream>

enum class Op { ACCEPT, READ, WRITE };

struct Conn {
    Op   op;
    int  fd;
    char buf[4096];
    int  len;
};

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

io_uring ring;

void submit_accept(int listen_fd, sockaddr_in* addr, socklen_t* len) {
    io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    io_uring_prep_accept(sqe, listen_fd, (sockaddr*)addr, len, 0);
    auto* c = new Conn{Op::ACCEPT, listen_fd, {}, 0};
    io_uring_sqe_set_data(sqe, c);
}


void submit_read(int fd) {
    io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    auto* c = new Conn{Op::READ, fd, {}, 0};
    io_uring_prep_read(sqe, fd, c->buf, sizeof(c->buf), 0);
    io_uring_sqe_set_data(sqe, c);
}

void submit_write(int fd, const char* data, int len) {
    io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    auto* c = new Conn{Op::WRITE, fd, {}, len};
    std::memcpy(c->buf, data, len);
    io_uring_prep_write(sqe, fd, c->buf, len, 0);
    io_uring_sqe_set_data(sqe, c);
}

int main() {
    const int PORT = 8080;
    int listen_fd = make_listen_socket(PORT);

    io_uring_queue_init(256, &ring, 0);

    sockaddr_in client_addr{};
    socklen_t   addr_len = sizeof(client_addr);
    submit_accept(listen_fd, &client_addr, &addr_len);
    io_uring_submit(&ring);

    std::cout << "Слушаем " << PORT << "\n";

    while (true) {
        io_uring_cqe* cqe;
        io_uring_wait_cqe(&ring, &cqe);

        Conn* c = static_cast<Conn*>(io_uring_cqe_get_data(cqe));
        int   res = cqe->res;

        switch (c->op) {
        case Op::ACCEPT: {
            int client_fd = res;
            submit_accept(listen_fd, &client_addr, &addr_len);
            if (client_fd >= 0)
                submit_read(client_fd);
            break;
        }
        case Op::READ: {
            if (res > 0) {
                submit_write(c->fd, c->buf, res);
            } else {
                close(c->fd);
            }
            break;
        }
        case Op::WRITE: {
            if (res >= 0) submit_read(c->fd);
            else          close(c->fd);
            break;
        }
        }

        delete c;
        io_uring_cqe_seen(&ring, cqe);
        io_uring_submit(&ring);
    }
}