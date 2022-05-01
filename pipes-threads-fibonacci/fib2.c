#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

//Sequence: 0, 1, 1, 2, 3, 5, 8, 13, 21, ...
//       n: 0  1  2  3  4  5  6   7   8  ...

int fib(int n) {
    if (n <= 1) {
        return n;
    }
    
    int n1;
    int n2;
    
    int fds[2][2];
    pipe(fds[0]);
    pipe(fds[1]);
    
    pid_t pid1 = fork();
    pid_t pid2 = fork();
    
    if (pid1 == 0) {
        n1 = fib(n - 1);
        write(fds[0][1], &n1, sizeof(int));
        exit(0);
    }
    
    if (pid2 == 0) {
        n2 = fib(n - 2);
        write(fds[1][1], &n2, sizeof(int));
        exit(0);
    }
    
    int wpid, status;
    while ((wpid = wait(&status)) > 0);
    
    read(fds[0][0], &n1, sizeof(int));
    read(fds[1][0], &n2, sizeof(int));
    
    return n1 + n2;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        exit(1);
    }

    int n = atoi(argv[1]);
    printf("Result: %d\n", fib(n));
}