#include "bufio.h"
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

