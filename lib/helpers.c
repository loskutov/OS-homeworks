#include "helpers.h"
#include <bufio.h>
#define __USE_XOPEN_EXTENDED
#define _XOPEN_SOURCE
#define __USE_POSIX
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

ssize_t read_(int fd, void *buf, size_t count) {
    ssize_t size;
    ssize_t offset = 0;
    do {
        size = read(fd, buf + offset, count);
        if (size == -1)
            return -1;
        offset += size;
        count  -= size;
    } while (size > 0);
    return offset;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    ssize_t size;
    ssize_t offset = 0;
    do {
        size = write(fd, buf + offset, count);
        if (size == -1)
            return -1;
        offset += size;
        count  -= size;
    } while (size > 0);
    return offset;
}

ssize_t read_until(int fd, void *buf, size_t count, char delimiter) {
    ssize_t size;
    size_t offset;
    for(offset = 0; offset < count; ++offset) {
        size = read(fd, buf + offset, 1);
        if (size == -1)
            return -1;
        if (size == 0)
            return offset;
        if (((char*)buf)[offset] == delimiter)
            break;
    }
    return offset + 1;
}


int spawn(const char* file, char* const argv[]) {
    int status;
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        execvp(file, argv);
        perror("execvp");
        return -1;
    } else if (wait(&status) == -1) {
        perror("wait");
        return -1;
    }
    if (!WIFEXITED(status)) {
        perror("WIFEXITED");
        return -1;
    }
    return WEXITSTATUS(status);
}


int exec(execargs_t* args) {
    return spawn(args->argv[0], args->argv);
}

execargs_t* create_execargs(char * const * argv) {
    execargs_t* ans = malloc(sizeof(execargs_t));
    if (!ans) {
        return NULL;
    }
    size_t count = 0;
    while (argv[count++]);
    ans->argv = malloc(count * sizeof(char*));
    ans->argv[--count] = NULL; 
    for (size_t i = 0; i < count; i++) {
        ans->argv[i] = strdup(argv[i]);
        if (!ans->argv[i]) {
            for (size_t j = 0; j < i; j++) {
                free(ans->argv[j]);
            }
            free(ans->argv);
            free(ans);
            return NULL;
        }
    }
    return ans;
}

void free_execargs(execargs_t* args) {
    for(char** i = args->argv; *i; i++) {
        free(*i);
    }
    free(args);
}

int runpiped(execargs_t** programs, size_t n) {
    int pipefd[n - 1][2];
    for (size_t i = 0; i < n - 1; i++) {
        if (pipe(pipefd[i]) == -1) {
            for (size_t j = 0; j < i; j++) {
                close(pipefd[i][0]);
                close(pipefd[i][1]);
            }
            return EXIT_FAILURE;
        }
    }
    pid_t pids[n];
    for (size_t i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            for (size_t j = 0; j < i; j++) {
                kill(pids[j], SIGKILL);
                waitpid(pids[j], NULL, 0);
            }
            return EXIT_FAILURE;
        }
        if (pids[i] == 0) {
            for (size_t j = 0; j < n - 1; j++) {
                if (j != i - 1)
                    close(pipefd[j][0]); 
                if (j != i)
                    close(pipefd[j][1]);
            }
            if (i != 0)
                dup2(pipefd[i - 1][0], STDIN_FILENO);
            if (i != n - 1)
                dup2(pipefd[i][1], STDOUT_FILENO);
            exec(programs[i]);
        }
    }
    for (size_t i = 0; i < n - 1; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }
    int status;
    waitpid(0, &status, 0);
    return WEXITSTATUS(status) == 0 ? 0 : -1;
}
