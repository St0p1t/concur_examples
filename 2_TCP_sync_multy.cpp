#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void handle_client(int client_fd, sockaddr_in client_addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
    std::cout << "Привет" << ip << ":" << ntohs(client_addr.sin_port) << "\n";

    char buf[1024];
    ssize_t n;
    while ((n = read(client_fd, buf, sizeof(buf))) > 0) {
        ssize_t total = 0;
        while (total < n) {
            ssize_t w = write(client_fd, buf + total, n - total);
            if (w <= 0) break;
            total += w;
        }
    }

    std::cout << "Пока" << ip << ":" << ntohs(client_addr.sin_port) << "\n";
    
    close(client_fd);
}

int main() {
    const int PORT = 8080;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }
    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    std::cout << "Ведется прослушка " << PORT << "\n";

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        std::thread t(handle_client, client_fd, client_addr);
        t.detach();
    }

    close(listen_fd);

    return 0;
}