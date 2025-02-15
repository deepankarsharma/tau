#include <stdio.h>
#include <stdlib.h>
#include "tau.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file-path>\n", argv[0]);
        return 1;
    }
    
    /* Open file */
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }
    
    /* Determine file size */
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(fp);
        return 1;
    }
    long filesize = ftell(fp);
    if (filesize < 0) {
        perror("ftell");
        fclose(fp);
        return 1;
    }
    rewind(fp);
    
    /* Allocate memory and read the file */
    char *buffer = malloc(filesize + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(fp);
        return 1;
    }
    size_t read_bytes = fread(buffer, 1, filesize, fp);
    buffer[read_bytes] = '\0';  /* Ensure null termination */
    fclose(fp);
    
    /* Create marker buffer and parse file contents */
    Buffer *buf = buffer_create(sizeof(Marker), 1024);
    if (!buf) {
        fprintf(stderr, "Error creating marker buffer\n");
        free(buffer);
        return 1;
    }
    if (read_markers(buffer, buf) != RETURN_STATUS_SUCCESS) {
        fprintf(stderr, "Error parsing markers\n");
        buffer_destroy(buf);
        free(buffer);
        return 1;
    }
    
    /* Print the markers */
    pretty_print_markers(buf, buffer);
    
    buffer_destroy(buf);
    free(buffer);
    return 0;
}
