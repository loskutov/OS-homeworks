#include <bufio.h>
#include <helpers.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>
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
    close(STDIN_FILENO);
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

    int q1[100500], head1 = 0, tail1 = 0;
    int q2[100500], head2 = 0, tail2 = 0;

    struct pollfd fds[256];
    fds[0].fd = sock1;
    fds[1].fd = sock2;
    fds[0].events = fds[1].events = POLLIN;

    for (unsigned accepted = 0; ;) {
        if (accepted >= 127)
            continue;
        puts("taks taks pollin");
        int events = poll(fds, 2 * accepted + 2, 100500);
        puts("taks taks polled)))");
        if (events == 0)
            break;
        if (fds[0].revents & POLLIN) {
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
            puts("mm k dvum podkluchilsya)))");
            ++accepted;
            fds[2 * accepted].fd = fd1;
            fds[2 * accepted + 1].fd = fd2;
            fds[2 * accepted].events = fds[2 * accepted + 1].events = POLLIN;
        }
        for (int i = 2; i < 256; i++) {
            if (fds[i].revents & POLLIN) {
                puts("taks tie)))");
                tie(fds[i].fd, fds[i ^ 1].fd);
                //tie(fds[i ^ 1].fd, fds[i].fd);
            }
        }
    }
    return EXIT_SUCCESS;
}
