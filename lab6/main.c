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
    int status;
    int sum = 0, prod = 1;

    int pid_first_child = fork();
    wait(&status);
    if (pid_first_child == 0) {
        // first child

        // spawn num subchildren
        for (int i = 1; i <= num; i++) {
            int pid_first_child_subprocess = fork();
            if (pid_first_child_subprocess == 0)
                exit(i);
            // make parent (first child) wait
            wait(&status);
            sum += WEXITSTATUS(status);
        }

        // exit from first child
        exit(sum);
        // NOTE: exit only accepts int parameters, so passing a value larger
        //  than the upper signed int limit (2^16 = 65536) will result in
        //  undefined behavior
    }

    // Get the first child's exit code
    printf("First child exited with: %d\n", WEXITSTATUS(status));

    // fork the second child
    int pid_second_child = fork();
    wait(&status);
    if (pid_second_child == 0) {
        // second child

        // spawn num subchildren
        for (int i = 1; i <= num; i++) {
            int pid_second_child_subprocess = fork();
            if (pid_second_child_subprocess == 0)
                exit(i);
            // make parent (second child) wait
            wait(&status);
            // status gets changed into the subprocess return code inside function

            prod *= WEXITSTATUS(status);
        }

        // exit from second child
        exit(prod);
        // NOTE: exit only accepts int parameters, so passing a value larger
        //  than the upper signed int limit (2^16 = 65536) will result in
        //  undefined behavior
    }

    // Get the first child's exit code
    printf("Second child exited with: %d\n", WEXITSTATUS(status));

    printf("Exited from parent");
    exit(0);

}