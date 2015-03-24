#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

ssize_t read_(int fd, void *buf, size_t count) {
    ssize_t size;
    ssize_t offset = 0;
    do {
        size = read(fd, buf + offset, count);
        if (size == -1)
            return -1;
        offset += size;
        count  -= size;
    } while (size > 0);
    return offset;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    ssize_t size;
    ssize_t offset = 0;
    do {
        size = write(fd, buf + offset, count);
        if (size == -1)
            return -1;
        offset += size;
        count  -= size;
    } while (size > 0);
    return offset;
}

ssize_t read_until(int fd, void *buf, size_t count, char delimiter) {
    ssize_t size;
    ssize_t offset;
    for(offset = 0; offset < count; ++offset) {
        size = read(fd, buf + offset, 1);
        if (size == -1)
            return -1;
        if (size == 0)
            return offset;
        if (((char*)buf)[offset] == delimiter)
            break;
    }
    return offset + 1;
}


int spawn(const char* file, char* const argv[]) {
    int status;
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        execvp(file, argv);
        perror("execvp");
        return -1;
    } else if (wait(&status) == -1) {
        perror("wait");
        return -1;
    }
    if (!WIFEXITED(status)) {
        perror("WIFEXITED");
        return -1;
    }
    return WEXITSTATUS(status);
}

