#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#define BYTES_PER_READ 100

int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "2 arguments are required: [source_file] [destination_file]\n");
        return 1;
    }

    char* file_1_path = argv[1];
    char* file_2_path = argv[2];

    int file_1 = open(file_1_path, O_RDONLY);
    if (file_1 < 0) {
        fprintf(stderr, "Error opening first file\n");
        return 1;
    }

    int file_2 = open(file_2_path, O_WRONLY | O_TRUNC);
    if (file_2 < 0) {
        fprintf(stderr, "Error opening second file\n");
        return 1;
    }

    char reading_buffer[BYTES_PER_READ];
    int bytes_read, bytes_written;

    while ((bytes_read = read(file_1, reading_buffer, BYTES_PER_READ)) > 0) {

        reading_buffer[bytes_read] = '\0';
        // Place string terminator at the end to avoid garbage characters

        bytes_written = write(file_2, reading_buffer, bytes_read);
        // NOTE: Must write exactly bytes_read, not BYTES_PER_READ!

        if (bytes_written < 0) {
            fprintf(stderr, "Error writing to second file\n");
            return 1;
        }

    }

    if (bytes_read < 0) {
        fprintf(stderr, "Error reading from first file\n");
        return 1;
    }

    printf("Operation successful.\n");

    return 0;
}