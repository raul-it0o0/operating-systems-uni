#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

/**
 * @file main.c
 * @brief A program that outputs a file's contents with `cat`,
 * executes the `wc` command on this output, and finally writes the output of `wc` in another file.
 */
int main(int argc, char* argv[]) {

    int n;
    int fd[2];

    if (argc != 3) {
        fprintf(stderr, "Error parsing arguments: <exec> <file1> <file2>");
        exit(1);
    }

    char *file1_path = argv[1];
    char *file2_path = argv[2];

    // open pipe
    if (pipe(fd) < 0) {
        fprintf(stderr, "Error opening pipe");
        exit(2);
    }

    // fork first child
    int pid_first_child = fork();
    if (pid_first_child == 0) {
        // close reading pipe
        close(fd[0]);

        if ((dup2(fd[1], STDOUT_FILENO)) < 0) {
            fprintf(stderr, "Error duplicating file1 to stdin");
            exit(3);
        }

        // since fd[1] and stdout now point to the same file,
        //  we can close fd[1]
        close(fd[1]);

        // call the cat program, with the first file as an argument
        // cat reads from file, and writes its content to stdout
        execlp("cat", "cat", file1_path, NULL);

        exit(0);
    } else if (pid_first_child < 0) {
        fprintf(stderr, "Error forking the first child");
        exit(4);
    }

    // NOTE: Not sure
    wait(NULL);

    // fork second child
    int pid_second_child = fork();
    if (pid_second_child == 0) {
        // close writing pipe
        close(fd[1]);

        if (dup2(fd[0], STDIN_FILENO) < 0) {
            fprintf(stderr, "Error duplicating file2 to stdin\n");
            exit(5);
        }

        // since fd[0] and stdin now point to the same file,
        //  we can close fd[0]
        close(fd[0]);

        int file_2_fd;
        if ((file_2_fd = open(file2_path, O_WRONLY)) < 0) {
            fprintf(stderr, "Error opening file to write in\n");
            exit(6);
        }

        // redirect stdout to file2 (stdout writes its contents to file2)
        if (dup2(file_2_fd, STDOUT_FILENO) < 0) {
            fprintf(stderr, "Error duplicating file2 to stdout\n");
            exit(7);
        }
        // close file_2_fd since stdout is already writing in it
        close(file_2_fd);

        execl("/bin/wc", "wc", NULL);
        // wc is called.
        //  it is called with execl because it's not found on the path
        //  wc waits for input
        //  this input comes from what the cat program (called in the first process)
        //  cat writes to stdout, but whatever got written in stdout got redirected to
        //  the writing pipe of fd

        exit(0);
    } else if (pid_second_child < 0) {
        fprintf(stderr, "Error forking the second child");
        exit(8);
    }

    // close before waiting!!
    //  if not, wc will deadlock waiting for input
    close(fd[0]);
    close(fd[1]);
    wait(NULL);

    // parent now prints the contents of file1 with cat
    execlp("cat", "cat", file2_path, NULL);

    exit(0);
}