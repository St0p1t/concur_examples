#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    const char* SERVER_IP = "127.0.0.1";
    const int   PORT = 8080;

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        std::cerr << "Неверный адрес\n";
        close(sock_fd);
        return 1;
    }

    if (connect(sock_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    std::cout << "Подключено\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) break;

        ssize_t sent = write(sock_fd, line.c_str(), line.size());
        if (sent <= 0) {
            perror("write");
            break;
        }

        char buf[1024];
        size_t total = 0;
        while (total < sizeof(buf)) {
            ssize_t n = read(sock_fd, buf + total, sizeof(buf) - total);
            if (n <= 0) {
                std::cout << "Сервер закрыл соединение\n";
                break;
            }
            total += n;
        }
        std::cout << "Ответ сервера: " << buf << "\n";
    }

    close(sock_fd);

    return 0;
}