#include <bufio.h>
#include <helpers.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#define BUF_SIZE 4096

int tie(int fd1, int fd2) {
    buf_t* buf = buf_new(BUF_SIZE);
    if (!buf) {
        perror("buf_new");
        return -1;
    }
    for(;;) {
        int offset = buf_fill(fd1, buf, 1);
        if (offset < 0) {
            buf_flush(fd2, buf, buf_size(buf));
            return -1;
        }
        if (offset == 0)
            break;
        if (buf_flush(fd2, buf, buf_size(buf)) < 0) {
            perror("buf_flush");
            return -1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port1> <port2>", argv[0]);
        return EXIT_FAILURE;
    }

    struct addrinfo * host1;
    struct addrinfo * host2;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };
    if (getaddrinfo("localhost", argv[1], &hints, &host1) ||
        getaddrinfo("localhost", argv[2], &hints, &host2)) {
        perror("getaddrinfo");
        return EXIT_FAILURE;
    }

    int sock1 = connect_to_host(host1);
    if (sock1 < 0) {
        perror("Couldn’t connect");
        return EXIT_FAILURE;
    }
    int sock2 = connect_to_host(host2);
    if (sock2 < 0) {
        perror("Couldn’t connect");
        return EXIT_FAILURE;
    }

    freeaddrinfo(host1);
    freeaddrinfo(host2);

    for(;;) {
        int fd1 = accept(sock1, NULL, NULL);
        if (fd1 < 0) {
            perror("accept");
            continue;
        }
        int fd2 = accept(sock2, NULL, NULL);
        if (fd2 < 0) {
            close(fd1);
            perror("accept");
            continue;
        }
        pid_t pid1 = fork();
        if (pid1 == -1) {
            close(fd1);
            close(fd2);
            perror("fork");
            continue;
        }

        if (pid1 == 0) {
            close(sock1);
            close(sock2);
            tie(fd1, fd2);
            return EXIT_SUCCESS;
        }

        pid_t pid2 = fork();
        if (pid2 == -1) {
            close(fd1);
            close(fd2);
            kill(pid1, SIGKILL);
            waitpid(pid1, NULL, 0);
            perror("fork");
            continue;
        }
        if (pid2 == 0) {
            close(sock1);
            close(sock2);
            tie(fd2, fd1);
            return EXIT_SUCCESS;
        }
        close(fd1);
        close(fd2);
    }
    return EXIT_SUCCESS;
}
