#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "A numeric argument is required.\n");
        return 1;
    }

    int num = atoi(argv[1]);
    // fork first child
    int pid1 = fork();
    // wait for first child to finish
    wait(NULL);

    if (pid1 == 0) {
        // block executed by first child process
        int sum = 0;
        for (int i = 1; i <= num; i++)
            sum = sum + i;
        printf("[ID = %d] Sum of positive integers up to %d is %d\n", getpid(), num, sum);
        exit(0);
    }
    else if (pid1 < 0) {
        // negative pid -> error
        fprintf(stderr, "Error creating child process.\n");
        return 1;
    }

    // fork second child
    int pid2 = fork();
    // wait for second child to finish
    wait(NULL);

    if (pid2 == 0) {
        // block executed by second child process
        int prod = 1;
        for (int i = 1; i <= num; i++)
            prod = prod * i;
        printf("[ID = %d] Factorial of %d is %d\n", getpid(), num, prod);
        exit(0);
    }
    else if (pid2 < 0) {
        // negative pid -> error
        fprintf(stderr, "Error creating child process.\n");
        return 1;
    }

    // print from parent
    printf("[ID = %d] Done\n", getpid());
    return 0;
}