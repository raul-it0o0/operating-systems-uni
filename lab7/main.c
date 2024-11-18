#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define BYTES_PER_READ 1024

void reverse_string(char* string);
void charstrcat(char* string, char character);
void set_information_string(char* information_string, char* string);

int main(int argc, char* argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Please specify a file path as argument!\n");
        return 1;
    }

    char* file_path = argv[1];

    // 0: input (read)
    // 1: output (write)
    // open all pipes that will be used in the beginning of the program
    int parent_to_first_child_pipe[2];
    int first_child_to_second_child_pipe[2];
    int second_child_to_parent_pipe[2];

    if (pipe(parent_to_first_child_pipe) < 0) {
        fprintf(stderr, "Error opening the first pipe\n");
        return 1;
    }

    if (pipe(first_child_to_second_child_pipe) < 0) {
        fprintf(stderr, "Error opening the first pipe");
        return 1;
    }

    if (pipe(second_child_to_parent_pipe) < 0) {
        fprintf(stderr, "Error opening the first pipe");
        return 1;
    }

    char reading_buffer[BYTES_PER_READ];
    int bytes_read = 0, bytes_written = 0;

    // fork first child
    int pid_parent = getpid();
    int pid_first_child = fork();
    if (pid_first_child == 0) {
        // first child

        // close writing channel
        close(parent_to_first_child_pipe[1]);

        // close other unused pipes
        close(first_child_to_second_child_pipe[0]);
        close(second_child_to_parent_pipe[0]);
        close(second_child_to_parent_pipe[1]);

        // read from first pipe and save into reading buffer
        memset(reading_buffer, '\0', sizeof(reading_buffer));
        while ((bytes_read = read(parent_to_first_child_pipe[0], reading_buffer, BYTES_PER_READ)) > 0) {
            // place string terminator at the end to avoid garbage characters
            reading_buffer[bytes_read] = '\0';
            // NOTE: since we only reverse the string once, we assume the contents of the file can be fit
            //  in reading_buffer (1024 characters)
        }
        if (bytes_read < 0) {
            fprintf(stderr, "Error reading from first pipe\n");
            return 1;
        }
        // close reading channel
        close(parent_to_first_child_pipe[0]);

        // process string (reverse it)
        reverse_string(reading_buffer);

        // write the reversed string in the writing channel of the second pipe
        if ((bytes_written = write(first_child_to_second_child_pipe[1], reading_buffer, strlen
        (reading_buffer))) < 0) {
            fprintf(stderr, "Error writing to second pipe\n");
            return 1;
        }
        // close writing channel of second pipe
        close(first_child_to_second_child_pipe[1]);

        // exit from first child
        exit(0);

    }
    else if (pid_first_child > 0) {
        // parent

        // close reading channel
        close(parent_to_first_child_pipe[0]);

        // closed unused channels
        close(first_child_to_second_child_pipe[1]);

        int file;
        if ((file = open(file_path, O_RDONLY)) < 0) {
            fprintf(stderr, "Error opening file\n");
            kill(0, SIGTERM);
        }

        memset(reading_buffer, '\0', sizeof(reading_buffer));
        while ((bytes_read = read(file, reading_buffer, BYTES_PER_READ)) > 0) {
            // place string terminator at the end to avoid garbage characters
            reading_buffer[bytes_read] = '\0';

            // gradually write what the parent read from the file, into the pipe
            bytes_written = write(parent_to_first_child_pipe[1], reading_buffer, bytes_read);
            close(parent_to_first_child_pipe[1]);

            if (bytes_written < 0) {
                fprintf(stderr, "Error writing to pipe 1\n");
                return 1;
            }
        }
        if (bytes_read < 0) {
            fprintf(stderr, "Error reading from file\n");
            return 1;
        }
        // finished writing to pipe, close it to signal the child to start reading
        close(parent_to_first_child_pipe[1]);
    }
    else {
        fprintf(stderr, "Error forking first child\n");
        return 1;
    }

    // wait for the first child to finish, in case it takes longer
    wait(NULL);
    // fork second child
    int pid_second_child = fork();
    if (pid_second_child == 0) {
        // second child

        // close unused writing output channel
        close(first_child_to_second_child_pipe[1]);

        // close other unused pipes
        close(parent_to_first_child_pipe[0]);
        close(parent_to_first_child_pipe[1]);
        close(second_child_to_parent_pipe[0]);

        // read from second pipe and save into reading buffer
        errno = 0;
        memset(reading_buffer, '\0', sizeof(reading_buffer));
        while ((bytes_read = read(first_child_to_second_child_pipe[0], reading_buffer, BYTES_PER_READ)) > 0) {
            // place string terminator at the end to avoid garbage characters
            reading_buffer[bytes_read] = '\0';
            // NOTE: since we only manipulate/process the string once, we assume the contents of the file
            //  can be fit in reading_buffer (1024 characters)
        }
        if (bytes_read < 0) {
            fprintf(stderr, "Error reading from second pipe (%s)\n", strerror(errno));
            kill(0, SIGTERM);
        }
        // close reading channel
        close(first_child_to_second_child_pipe[0]);

        // process string
        char information_string[strlen(reading_buffer)];
        memset(information_string, '\0', sizeof(information_string));
        set_information_string(information_string, reading_buffer);

        // write to the writing channel of the third pipe
        if ((bytes_written = write(second_child_to_parent_pipe[1], information_string, strlen
                (information_string))) < 0) {
            fprintf(stderr, "Error writing to third pipe\n");
            kill(0, SIGTERM);
        }
        // close writing channel of third pipe
        close(second_child_to_parent_pipe[1]);

        // exit from second child
        exit(0);

    }
    else if (pid_second_child > 0) {
        wait(NULL);
        // wait for the second child to finish writing to pipe
        // parent

        // close unused channels
        close(parent_to_first_child_pipe[0]);
        close(parent_to_first_child_pipe[1]);
        close(first_child_to_second_child_pipe[0]);
        close(first_child_to_second_child_pipe[1]);
        close(second_child_to_parent_pipe[1]);

        // read from third pipe and print to screen
        memset(reading_buffer, '\0', sizeof(reading_buffer));
        while ((bytes_read = read(second_child_to_parent_pipe[0], reading_buffer, BYTES_PER_READ)) > 0) {
            // place string terminator at the end to avoid garbage characters
            reading_buffer[bytes_read] = '\0';
            // NOTE: since we only manipulate/process the string once, we assume the contents of the file
            //  can be fit in reading_buffer (1024 characters)
        }
        if (bytes_read < 0) {
            fprintf(stderr, "Error reading from third pipe\n");
            kill(0, SIGTERM);
        }
        // close reading channel
        close(first_child_to_second_child_pipe[0]);

        printf("%s\n", reading_buffer);

        return 0;
    }
}

void reverse_string(char* string) {
    int first = 0;
    int last = strlen(string) - 1;

    while (first < last) {
        char temp = string[first];
        string[first] = string[last];
        string[last] = temp;

        first++;
        last--;
    }
}

void set_information_string(char* information_string, char* string) {
    char buf_digits[strlen(string)];
    char buf_uppercase[strlen(string)];
    char buf_lowercase[strlen(string)];
    char buf_misc[strlen(string)];

    memset(buf_digits, '\0', sizeof(buf_digits));
    memset(buf_uppercase, '\0', sizeof(buf_uppercase));
    memset(buf_lowercase, '\0', sizeof(buf_lowercase));
    memset(buf_misc, '\0', sizeof(buf_misc));

    for (int i = 0; i < strlen(string); i++) {
        if (string[i] >= '0' && string[i] <= '9')
            charstrcat(buf_digits, string[i]);
        else if (string[i] >= 'A' && string[i] <= 'Z')
            charstrcat(buf_uppercase, string[i]);
        else if (string[i] >= 'a' && string[i] <= 'z')
            charstrcat(buf_lowercase, string[i]);
        else 
            charstrcat(buf_misc, string[i]);
    }
    sprintf(information_string, "%lu%s%lu%s%lu%s%lu%s",
           strlen(buf_digits), buf_digits, strlen(buf_uppercase), buf_uppercase, strlen(buf_lowercase),
           buf_lowercase, strlen(buf_misc), buf_misc);

}

void charstrcat(char* string, char character) {
    char temp_string[2];
    temp_string[0] = character;
    temp_string[1] = '\0';

    strcat(string, temp_string);
}