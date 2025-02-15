#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "tau.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0)
        return 0;
    
    /* Create a null-terminated copy of the input */
    char *input = malloc(size + 1);
    if (!input)
        return 0;
    memcpy(input, data, size);
    input[size] = '\0';
    
    Buffer *buf = buffer_create(sizeof(Marker), 1024);
    if (!buf) {
        free(input);
        return 0;
    }
    
    /* Call read_markers; any crash or memory error in read_markers will be caught */
    read_markers(input, buf);
    
    buffer_destroy(buf);
    free(input);
    return 0;
}
