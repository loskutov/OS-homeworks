#include <bufio.h>
#include <helpers.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define BUF_SIZE 4096

void print_line(int sig) {
    write_(STDOUT_FILENO, "\n", 1);
}

int main(int argc, char *argv[]) {
    char buf[BUF_SIZE];

    buf_t* buffer = buf_new(BUF_SIZE);
    signal(SIGINT, print_line);

    for (;;) {
        write_(STDOUT_FILENO, "$", 1);
        ssize_t res = buf_getline(STDIN_FILENO, buffer, buf);
        puts(buf);
        if (res < 0)
            return EXIT_FAILURE;
        if (res == 0)
            return EXIT_SUCCESS;
        buf[res] = '\0';
        int programs = 1;
        for (ssize_t i = 0; i < res; i++) {
            if (buf[i] == '|') {
                programs++;
                buf[i] = '\0';
            }
        }

        execargs_t* e[programs];
        ssize_t last_pos = 0;
        int cnt = 0;
        for (ssize_t i = 0; i <= res; i++) {
            if (buf[i] == 0) {
                int args = (buf[i - 1] != ' ');
                ssize_t last_space = last_pos - 1;
                for (ssize_t j = last_pos; j < i; j++) {
                    if (buf[j] == ' ') {
                        if (last_space != j - 1) {
                            args++;
                        }
                        buf[j] = 0;
                        last_space = j;
                    }
                }
                char* proc_args[args + 1];
                proc_args[args] = NULL;
                ssize_t last_arg = last_pos;
                int arg_cnt = 0;
                for (ssize_t j = last_pos; j <= i; j++) {
                    if (buf[j] == '\0') {
                        if (last_arg != j) {
                            proc_args[arg_cnt++] = buf + last_arg;
                        }
                        last_arg = j + 1;
                    }
                }

                if (!(e[cnt++] = create_execargs(proc_args))) {
                    for (int j = 0; j < cnt - 1; j++) {
                        free_execargs(e[j]);
                    }
                    perror("create_execargs");
                    break;
                }
                last_pos = i + 1;
            }
        }

        if (runpiped(e, programs)) {
            perror("runpiped");
        }

        for (int i = 0; i < programs; i++) {
            free_execargs(e[i]);
        }
    }
}
