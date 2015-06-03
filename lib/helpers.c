#define __USE_XOPEN_EXTENDED
#define __USE_POSIX
#define _XOPEN_SOURCE
#define _POSIX_SOURCE
#define _GNU_SOURCE
#include "helpers.h"
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

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
    return execvp(**args, *args);//spawn(args->argv[0], args->argv);
}

execargs_t create_execargs(char * const * argv) {
    int cnt;
    for (cnt = 0; argv[cnt]; cnt++);
    execargs_t ans = malloc(cnt * sizeof(char*));
    for (int i = 0; i < cnt; i++) {
        ans[i] = strdup(argv[i]);
    }
    return argv;
    //execargs_t* ans = malloc(sizeof(execargs_t));
    //if (!ans) {
        //return NULL;
    //}
    //size_t count = 0;
    //while (argv[count++]);
    //ans->argv = malloc(count * sizeof(char*));
    //ans->argv[--count] = NULL; 
    //for (size_t i = 0; i < count; i++) {
        //ans->argv[i] = strdup(argv[i]);
        //if (!ans->argv[i]) {
            //for (size_t j = 0; j < i; j++) {
                //free(ans->argv[j]);
            //}
            //free(ans->argv);
            //free(ans);
            //return NULL;
        //}
    //}
    //return ans;
}

void free_execargs(execargs_t args) {
    for(char** i = args/* ->argv*/; *i; i++) {
        //free(*i);
    }
    //free(*args);
    //free(args/* ->argv */);
}

int runpiped(execargs_t** args, size_t n) {
    int pipes[2 * (n - 1)];
    for (size_t i = 1; i < n; i++) {
        if (pipe2(pipes + 2 * (i - 1), O_CLOEXEC)) {
            for (int j = 1; j < i; j++) {
                close(pipes[j * 2 - 2]);
                close(pipes[j * 2 - 1]);
            }
            return -1;
        }
    }

    pid_t pids[n];
    memset(pids, 0, n * sizeof(pid_t));

    sigset_t mask;
    sigset_t original_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &original_mask);

    int error = 0;
    for (size_t i = 0; i < n; i++) {
        int pid = fork();
        if (pid < 0) {
            error = 1;
            break;
        } else if (pid) {
            pids[i] = pid;
        } else {
            if (i > 0) dup2(pipes[i * 2 - 2], STDIN_FILENO);
            if (i < n - 1) dup2(pipes[i * 2 + 1], STDOUT_FILENO);
            exec(args[i]);
            exit(-1);
        }
    }
    
    for (size_t i = 1; i < n; i++) {
        close(pipes[i * 2 - 2]);
        close(pipes[i * 2 - 1]);
    }   

    if (error) {
        for (int i = 0; i < n; i++) {
            if (pids[i]) {
                kill(pids[i], SIGKILL);
                waitpid(pids[i], 0, 0);
            }
        }
        sigprocmask(SIG_SETMASK, &original_mask, 0);
        return -1;
    }

    siginfo_t info;
    int killed = 0;
    int killed_all = 0;
    while (!killed_all) {
        sigwaitinfo(&mask, &info);
        if (info.si_signo == SIGINT) {
            break;
        }

        if (info.si_signo == SIGCHLD) {
            int chld;
            while ((chld = waitpid(-1, 0, WNOHANG)) > 0) {
                for (int i = 0; i < n; i++) {
                    if (pids[i] == chld) {
                        pids[i] = 0;
                        break;
                    }
                }
                killed++;
                if (killed == n) {
                    killed_all = 1;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < n; i++) {
        if (pids[i]) {
            kill(pids[i], SIGKILL);
            waitpid(pids[i], 0, 0); 
        }
    }

    sigprocmask(SIG_SETMASK, &original_mask, 0);
    return 0;
}

int runpiped_my(execargs_t** programs, size_t n) {
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


int connect_to_host(struct addrinfo * host) {
    int sock;

    struct addrinfo * rp;
    for (rp = host; rp; rp = rp->ai_next) {
        int one = 1;
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == 0
                && bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sock);
    }
    if (!rp)
        return -1;
    if (listen(sock, 32)) {
        close(sock);
        perror("listen");
        return -1;
    }

    return sock;
}

