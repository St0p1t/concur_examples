#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../10_busy_loop_polling.cpp"

// Busy-loop polling: неблокирующий accept + неблокирующий read в одном цикле.
// Проблемы: 100% CPU даже при отсутствии соединений; нет засыпания на poll/epoll.
// Запуск: ./demo_10, тест: echo "hello" | nc -q 1 127.0.0.1 8080
int main() {
    const int PORT = 8080;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);
    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, SOMAXCONN);

    set_nonblocking(listen_fd);

    std::cout << "Busy-loop echo server on port " << PORT << "\n";
    std::cout << "Test: echo \"hello\" | nc -q 1 127.0.0.1 8080\n";

    busy_loop_server(listen_fd);
    return 0;
}
