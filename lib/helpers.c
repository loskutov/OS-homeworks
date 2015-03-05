#include "helpers.h"

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
