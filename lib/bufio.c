#include "bufio.h"
#include <stdbool.h>
#include <stdio.h>
#ifdef DEBUG
#define FALL_IF_NULL(buffer) \
    if (!(buffer)) abort;
#else
#define FALL_IF_NULL(a)
#endif

buf_t *buf_new(size_t capacity) {
    buf_t* buffer = (buf_t*)malloc(sizeof(buf_t));
    if (!buffer)
        return NULL;
    buffer->data = (char*)malloc(capacity);
    if (!buffer->data) {
        free(buffer);
        return NULL;
    }
    buffer->capacity = capacity;
    buffer->size = 0;
    return buffer;
}

void buf_free(buf_t *buffer) {
    FALL_IF_NULL(buffer);
    free(buffer->data);
    free(buffer);
}

size_t buf_capacity(buf_t *buffer) {
    FALL_IF_NULL(buffer);
    return buffer->capacity;
}

size_t buf_size(buf_t *buffer) {
    FALL_IF_NULL(buffer);
    return buffer->size;
}

ssize_t buf_fill(int fd, buf_t *buffer, size_t required) {
    FALL_IF_NULL(buffer);
    ssize_t n = -1;
    while (n != 0 && buffer->size < required) {
        n = read(fd, buffer->data + buffer->size, buffer->capacity - buffer->size);
        if (n == -1)
            return -1;
        buffer->size += n;
    }
    return buffer->size;
}

ssize_t buf_flush(int fd, buf_t* buffer, size_t required) {
    FALL_IF_NULL(buffer);
    ssize_t n;
    size_t offset = 0;
    while (offset < required && offset < buffer->size) {
        n = write(fd, buffer->data + offset, buffer->size - offset);
        if (n == -1)
            return -1;
        offset += n;
    }
    if (offset < buffer->size)
        memmove(buffer->data, buffer->data + offset, buffer->size - offset);
    buffer->size -= offset;
    return offset;
}

size_t buf_getline(int fd, buf_t* buf, char* dest) {
    int oldpos = 0;
    int pos;
    int dest_offset = 0;
    do {
        for (pos = oldpos; pos < buf->size; pos++) {
            if (buf->data[pos] == '\n') {
                break;
            }
        }
        if (pos == buf->size)
            memcpy(dest + dest_offset, buf->data + oldpos, pos - oldpos);
        else
            memcpy(dest + dest_offset, buf->data + oldpos, pos - oldpos + 1);

        dest_offset += pos - oldpos;
        if (pos == buf->size) {
            ssize_t res = buf_fill(fd, buf, 1);
            if (res == -1) {
                perror("buf_fill");
            } else if (res == 0) {
                dest[dest_offset] = 0;
                return dest_offset;
            }
        } else break;
    } while (true);
    if (pos != buf->size)
        memmove(buf->data, buf->data + pos + 1, buf->size - pos - 1);
    buf->size -= pos + 1;
    dest[dest_offset] = 0;
    return dest_offset;
}

ssize_t buf_write(int fd, buf_t* buf, char* src, size_t len) {
    puts("gonna write");
    int old_len = len;
    int written = 0;
    while (true) {
        if (buf->capacity - buf->size < len) {
            memcpy(buf->data + buf->size, src + written, buf->capacity - buf->size);
            len -= buf->capacity - buf->size;
            written += buf->capacity - buf->size;
            buf->size = buf->capacity;
            puts("gonna flush");
            buf_flush(fd, buf, 1);
            puts("flush");
        } else {
            memcpy(buf->data + buf->size, src + written, len);
            buf->size += len;
            puts("written");
            return old_len;
        }
    }
}
