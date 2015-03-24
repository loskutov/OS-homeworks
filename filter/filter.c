#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <helpers.h>
#include "filter.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        puts("Usage: filter <file>");
        return EXIT_FAILURE;
    }


    char buf[8192];

    char* arguments[argc + 1];

    memcpy(arguments, argv + 1, (argc - 1) * sizeof(char*));
    arguments[argc - 1] = buf;
    arguments[argc] = NULL;

    ssize_t n;
    while ((n = read_until(STDIN_FILENO, buf, sizeof(buf), '\n')) != 0) {
        if (n == -1) {
            perror("Error while reading");
            return EXIT_FAILURE;
        } else {
            if (buf[n - 1] == '\n')
                buf[n - 1] = '\0';
            else
                buf[n] = '\0';
            if (spawn(arguments[0], arguments) == 0)
                puts(buf);
        }
    }
    return EXIT_SUCCESS;
}
