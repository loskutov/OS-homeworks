#include <helpers.h>
#include <bufio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BUF_SIZE 4096

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port> <file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct addrinfo * host;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    }; // all the other fields are supposed to be initialized
       // with the default values (zeros)
    if (getaddrinfo("localhost", argv[1], &hints, &host)) {
        perror("getaddrinfo");
        return EXIT_FAILURE;
    }

    int sock = connect_to_host(host);

    if (sock == -1) {
        write_(STDERR_FILENO, "Couldn’t connect\n", 19);
        return EXIT_FAILURE;
    }

    freeaddrinfo(host);

    for(;;) {
        struct sockaddr_in client;
        socklen_t sz = sizeof(client);
        int sock_fd = accept(sock, (struct sockaddr *)&client, &sz);
        if (sock_fd == -1) {
            perror("accept");
            continue;
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
        } else if (pid == 0) {
            close(sock);
            int file_fd = open(argv[2], O_RDONLY);
            buf_t *buf = buf_new(BUF_SIZE);
            if (buf == NULL) {
                perror("buf_new");
                return EXIT_FAILURE;
            }

            for(;;) {
                int offset = buf_fill(file_fd, buf, 1);
                if (offset < 0) {
                    buf_flush(sock_fd, buf, buf_size(buf));
                    return EXIT_FAILURE;
                } else if (offset == 0)
                    return EXIT_SUCCESS;

                if (buf_flush(sock_fd, buf, buf_size(buf)) < 0)
                    return EXIT_FAILURE;
            }
        } else {
            close(sock_fd);
        }
    }
    return EXIT_SUCCESS;
}
