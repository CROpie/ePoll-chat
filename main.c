#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void) {
    // Create the listening socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Allow reuse of the port immediately after the process exits
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to PORT on all interfaces
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));

    // Start listening (backlog of 10)
    listen(server_fd, 10);
    printf("Listening on port %d\n", PORT);

    // Accept one connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

    // Echo loop
    char buf[BUFFER_SIZE];
    int bytes;
    while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
        send(client_fd, buf, bytes, 0);
    }

    printf("Client disconnected\n");
    close(client_fd);
    close(server_fd);
    return 0;
}

