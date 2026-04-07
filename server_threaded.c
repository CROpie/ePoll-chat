#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buf[BUFFER_SIZE];
    int bytes;
    while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
        send(client_fd, buf, bytes, 0);
    }

    printf("Client %d disconnected\n", client_fd);
    close(client_fd);
    return NULL;
}

int main(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));

    listen(server_fd, 10);
    printf("Listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        printf("Client connected: %s (fd=%d)\n", inet_ntoa(client_addr.sin_addr), client_fd);

        // Give each client its own thread
        pthread_t thread;
        int *fd_ptr = malloc(sizeof(int));
        *fd_ptr = client_fd;
        pthread_create(&thread, NULL, handle_client, fd_ptr);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
