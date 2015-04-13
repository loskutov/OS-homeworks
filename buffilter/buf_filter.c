#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <bufio.h>
#include <helpers.h>
#include "buf_filter.h"

int main(int argc, char* argv[]) {
    char str[8192];
    buf_t* buf = buf_new(8192);
    if (argc < 2) {
        puts("Usage: filter <file>");
        return EXIT_FAILURE;
    }


    char* arguments[argc + 1];

    memcpy(arguments, argv + 1, (argc - 1) * sizeof(char*));
    arguments[argc - 1] = str;
    arguments[argc] = NULL;

    ssize_t n;
    while ((n = buf_getline(STDIN_FILENO, buf, str)) != 0) {
        if (n == -1) {
            perror("Error while reading");
            return EXIT_FAILURE;
        } else {
            str[n+1] = '\0';
            if (str[n] == '\n')
                str[n] = '\0';
            if (spawn(arguments[0], arguments) == 0) {
                int j = buf_write(STDOUT_FILENO, buf, str, n);
            }
        }
    }
    buf_flush(STDOUT_FILENO, buf, buf->size);
    return EXIT_SUCCESS;
}
