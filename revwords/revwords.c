#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <helpers.h>

void swap(char* a, char* b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

char* reverse(char* buf, size_t size) {
    for (size_t i = 0; i < size/2; ++i) {
        swap(buf + i, buf + size - i - 1);
    }
    return buf;
}

int main() {
    char buf[8192];
    ssize_t n;
    while ((n = read_until(STDIN_FILENO, buf, sizeof(buf), ' ')) != 0) {
        if (n == -1) {
            perror("Error while reading");
            return EXIT_FAILURE;
        } else {
            int no_trail = n - (buf[n - 1] == ' ');
            if (write_(STDOUT_FILENO, reverse(buf, no_trail), n) == -1) {
                perror("Error while writing");
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
