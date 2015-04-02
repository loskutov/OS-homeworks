#include <unistd.h>
#include <bufio.h>

int main() {
    buf_t *buf = buf_new(4096);
    int fill_res, flush_res;
    do {
        fill_res = buf_fill(STDIN_FILENO, buf, 1);
        flush_res = buf_flush(STDOUT_FILENO, buf, buf_size(buf));
        if (fill_res == -1 || flush_res == -1) {
            perror("IO");
            return EXIT_FAILURE;
        }
    } while (flush_res != 0);
    return EXIT_SUCCESS;
}
