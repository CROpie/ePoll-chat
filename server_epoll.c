#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 64

void set_nonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
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
    set_nonblocking(server_fd);
    printf("Listening on port %d\n", PORT);

    // Create the epoll instance
    int epfd = epoll_create1(0);

    // Register the listening socket with epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    struct epoll_event events[MAX_EVENTS];

    while (1) {
        // Block until one or more fds are ready
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                // New connection — accept and add to epoll
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                set_nonblocking(client_fd);
                printf("Client connected: %s (fd=%d)\n", inet_ntoa(client_addr.sin_addr), client_fd);

                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

            } else {
                // Existing client has data
                char buf[BUFFER_SIZE];
                int bytes = recv(fd, buf, sizeof(buf), 0);

                if (bytes <= 0) {
                    // 0 = clean disconnect, -1 = error
                    printf("Client %d disconnected\n", fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    send(fd, buf, bytes, 0);
                }
            }
        }
    }

    close(server_fd);
    close(epfd);
    return 0;
}
