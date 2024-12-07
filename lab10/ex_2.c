#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#define SIGNAL_SEND_LIMIT 10

// allows the value of interrupted to be changed
// from within a signal handler
volatile sig_atomic_t interrupted;

void signal_handler(int signal) {
    interrupted = 1;
}

int main(int argc, char* argv[]) {

    // Parse argument (N/quantum)
    if (argc != 2) {
        fprintf(stderr, "Error parsing argument: <exec> <N>\n");
        exit(1);
    }

    int quantum = strtol(argv[1], NULL, 10);

    // NOTE: If we want to be 100% accurate, we would have to track the time beginning from this point
    //  (before forking), wait <quantum> seconds and then interrupt processes

    int first_child_pid = fork();
    if (first_child_pid < 0) {
        fprintf(stderr, "Error forking first child\n");
        exit(1);
    }
    else if (first_child_pid == 0) {
        // first child
        // ignore SIGUSR1 signal
        signal(SIGUSR1, SIG_IGN);
        while(1);
        // wait for signals
    }
    else {
        // parent
        int second_child_pid = fork();
        if (second_child_pid < 0) {
            fprintf(stderr, "Error forking second child");
            exit(1);
        }
        else if (second_child_pid == 0) {
            // second child
            signal(SIGUSR1, SIG_IGN);
            while(1);
            // wait for signals
        }
        else {
            interrupted = 0;

            signal(SIGUSR1, signal_handler);

            // to send a SIGUSR1 signal to the parent:
                // get parent pid with getpid()
             printf("%d\n", getpid());
                // open another terminal and execute:
                // kill -SIGUSR1 <parent pid>

            while (!interrupted) {
                // Wait <quantum> seconds
                sleep(quantum);
                // Interrupt first subprocess, let second subprocess run
                printf("Process 1 was put on hold, process 2 is still running\n");
                kill(first_child_pid, SIGSTOP);
                break;
            }

            while (!interrupted) {
                // Wait <quantum> seconds
                sleep(quantum);
                if (interrupted) break;
                // Interrupt second subprocess, let first subprocess run
                kill(second_child_pid, SIGSTOP);
                if (interrupted) break;
                kill(first_child_pid, SIGCONT);
                if (interrupted) break;
                printf("Process 2 was put on hold, process 1 started\n");

                // check if interrupted (again)
                if (interrupted) break;

                // Wait <quantum> seconds
                sleep(quantum);
                if (interrupted) break;
                // Interrupt first subprocess, resume second subprocess
                kill(first_child_pid, SIGSTOP);
                if (interrupted) break;
                kill(second_child_pid, SIGCONT);
                if (interrupted) break;
                printf("Process 1 was put on hold, process 2 started\n");
            }

            // SIGUSR received, terminate both processes with SIGTERM
            // first resume both processes, since they cannot be terminated if they are stopped
            kill(first_child_pid, SIGCONT);
            kill(second_child_pid, SIGCONT);
            // then terminate both processes
            kill(first_child_pid, SIGTERM);
            kill(second_child_pid, SIGTERM);

            // wait for both children to terminate
            waitpid(first_child_pid, NULL, 0);
            waitpid(second_child_pid, NULL, 0);

            // optionally print message before exiting
            printf("Got SIGUSR1\n");
            // NOTE: the subprocesses can be checked if they are still running with:
            //  ps aux | grep ex_2
            return 0;
        }
    }
}

