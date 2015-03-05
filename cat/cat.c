#include <stdlib.h>
#include <helpers.h>

int main() {

    char buf[4096];
    ssize_t n;
    while ((n = read_(STDIN_FILENO, buf, sizeof(buf))) != 0) {
        if (n == -1) {
            perror("Error while reading");
            return EXIT_FAILURE;
        } else if (write_(STDOUT_FILENO, buf, n) == -1) {
            perror("Error while writing");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
