#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <helpers.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        puts("Usage: filter <file>");
        return -1;
    }

    const char* file = argv[1];

    char buf[8192];

    argv[argc] = buf;
    argv[argc + 1] = NULL;
    ssize_t n;
    while ((n = read_until(STDIN_FILENO, buf, sizeof(buf), '\n')) != 0) {
        if (n == -1) {
            perror("Error while reading");
            return EXIT_FAILURE;
        } else {
            if (buf[n - 1] == '\n')
                buf[n - 1] = '\0';
            if (spawn(file, argv + 1) == 0)
                puts(buf);
        }
    }
    return EXIT_SUCCESS;
}
