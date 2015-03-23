#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <helpers.h>


int main() {
    char buf[8192];
    char ans[8]; // should be enough for everyone
    const char delimiter = ' ';
    ssize_t n;
    while ((n = read_until(STDIN_FILENO, buf, sizeof(buf), delimiter)) != 0) {
        if (n == -1) {
            perror("Error while reading");
            return EXIT_FAILURE;
        } else {
            n -= (buf[n - 1] == delimiter);
            n = sprintf(ans, "%zi ", n);
            if (write_(STDOUT_FILENO, ans, n) == -1) {
                perror("Error while writing");
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
