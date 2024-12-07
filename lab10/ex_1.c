#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#define SIGNAL_SEND_LIMIT 10
#define MAX_PRINTS 2

volatile sig_atomic_t n = 0;
double a[SIGNAL_SEND_LIMIT];

/**
 * @brief Format val as a string and store it in string.
 * @param val
 * @param string
 */
void double_to_str_signal_safe(double val, char string[]) {
    // extract integer part
    int int_part = (int) val;

    // val = decimal part
    val = val - int_part;

    // extract integer part and add to string
    int i = 0;
    char temp[50];
    while (int_part > 0) {
        temp[i] = '0' + (int_part % 10);
        int_part = int_part / 10;
        i++;
    }

    // reverse integer part
    int j = i;
    while (j > 0) {
        string[i - j] = temp[j-1];
        j--;
    }



    if (val != 0) {
        // add decimal point
        string[i] = '.';
        // extract decimal part and add to string
        for (int j = 0; j < 3; j++) {  // only write 2 decimal points
            val *= 10;
            int digit = (int) val;
            string[i+1] = '0' + digit;
            i++;
            val -= digit;
        }
    }

    string[i] = '\0';
}

void signal_handler(int signal) {
    char buf[50];
    double_to_str_signal_safe(a[n], buf);
    write(STDOUT_FILENO, strcat(buf, "\n"), strlen(buf)+2);
    n++;
}

/**
 * @param a0 The initial value of the a sequence
 * @param r The denominator of the fraction that is added to each sequence member
 *
 * Create a subprocess, send a SIGUSR1 signal every second.
 * Firstly print '*****', and then
 * each second alternate between printing '+++++' and '------'.
 * <br>
 * Have the subprocess constantly compute a[n+1] = a[n] + 1/r, where a[0] = a0.
 * <br>
 * Upon receiving the SIGUSR signal, the subprocess prints the most recently computed value
 * of the sequence a[n], and increments the value n
 * to have the subprocess compute the next element in the sequence.
 * <br>
 * The subprocess ends after it has received a number of SIGUSR signals equal to MAX_PRINTS (see code).
 * <br>
 * Get the subprocess' exit code and print it.
 */
int main(int argc, char* argv[]) {

    // Parse arguments
    if (argc != 3) {
        fprintf(stderr, "Error parsing arguments: <exec> <a0> <r>\n");
        exit(1);
    }

    double a0 = strtod(argv[1], NULL);
    double r = strtod(argv[2], NULL);

    // Have the parent ignore the SIGUSR1 signal
    signal(SIGUSR1, SIG_IGN);

    int first_child_pid = fork();
    if (first_child_pid < 0) {
        fprintf(stderr, "Error forking first child\n");
        exit(1);
    }
    else if (first_child_pid == 0) {
        // first child

        // set sigusr1_handler as the handler function for SIGUSR1 signal
        if (signal(SIGUSR1, signal_handler) < 0) {
            fprintf(stderr, "Error setting handler of SIGUSR1\n");
        }

        // compute (once)
        a[0] = a0;

        // standby, wait for signal(s)
        // while on standby,
        while(1) {
            // check how many times SIGUSR1 has been received by child
            // stop at MAX_PRINTS signals received (MAX_PRINTS prints)
            if (n == MAX_PRINTS)
                break;

            // keep recomputing a[n+1]
            a[n+1] = a[n] + 1/r;
        }

        exit(0);
    }
    else {
        // parent

        int status;

        // first print (once)
        printf("*****\n");

        for(int i = 1;;i++) {
            sleep(1);
            if (kill(first_child_pid, SIGUSR1) < 0) {
                fprintf(stderr, "Error sending SIGUSR1 signal to child\n");
            }
            i % 2 == 0 ? printf("+++++") : printf("------");
            // printf(" (second %d)", i);
            printf("\n");
            // check if child has exited, if it hasn't then don't wait
            if (waitpid(first_child_pid, &status, WNOHANG) == first_child_pid)
                break;
        }

        printf("Child exited with %d\n", WEXITSTATUS(status));
        return 0;
    }
}
