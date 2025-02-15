#include "tau.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* 
 * X-Macro for fixed marker tokens.
 * Order matters: longer tokens must come before shorter ones.
 */
#define MARKER_TOKENS(X)                                  \
    X(MARKER_UNSYNTAX_SPLICING, "#,@", 3)                 \
    X(MARKER_NIL,              "nil", 3)                  \
    X(MARKER_UNQUOTE_SPLICING, ",@", 2)                   \
    X(MARKER_SYNTAX,            "#'", 2)                  \
    X(MARKER_QUASI_SYNTAX,      "#`", 2)                  \
    X(MARKER_UNSYNTAX,          "#,", 2)                  \
    X(MARKER_TRUE,              "#t", 2)                  \
    X(MARKER_FALSE,             "#f", 2)                  \
    X(MARKER_LPAREN,            "(",  1)                  \
    X(MARKER_RPAREN,            ")",  1)                  \
    X(MARKER_QUOTE,             "'",  1)                  \
    X(MARKER_QUASI_QUOTE,       "`",  1)                  \
    X(MARKER_UNQUOTE,           ",",  1)


/* Create a new buffer with a given element size and initial capacity. */
Buffer* buffer_create(size_t element_size, size_t initial_capacity) {
    Buffer *buf = malloc(sizeof(Buffer));
    if (!buf) return NULL;

    buf->data = malloc(element_size * initial_capacity);
    if (!buf->data) {
        free(buf);
        return NULL;
    }
    buf->element_size = element_size;
    buf->capacity = initial_capacity;
    buf->count = 0;
    return buf;
}

/* Free the memory used by the buffer. */
void buffer_destroy(Buffer *buf) {
    if (buf) {
        free(buf->data);
        free(buf);
    }
}


void buffer_cleanup(Buffer **buf) {
    buffer_destroy(*buf);
}

/* Resize the buffer to a new capacity. Returns 1 on success, 0 on failure. */
int buffer_resize(Buffer *buf, size_t new_capacity) {
    void *new_data = realloc(buf->data, buf->element_size * new_capacity);
    if (!new_data) return 0;
    buf->data = new_data;
    buf->capacity = new_capacity;
    return 1;
}

/* 
 * Push a new element onto the end of the buffer.
 * Returns 1 on success and 0 on failure.
 */
int buffer_push(Buffer *buf, const void *element) {
    if (buf->count == buf->capacity) {
        size_t new_capacity = (buf->capacity == 0) ? 1 : buf->capacity * 2;
        if (!buffer_resize(buf, new_capacity)) {
            return 0; // Resize failed
        }
    }
    void *target = (char*)buf->data + (buf->count * buf->element_size);
    memcpy(target, element, buf->element_size);
    buf->count++;
    return 1;
}

/* 
 * Returns a pointer to the nth element (0-indexed) or NULL if out of bounds.
 */
void* buffer_nth(Buffer *buf, size_t n) {
    if (n >= buf->count) {
        return NULL; // Out of bounds
    }
    return (char*)buf->data + (n * buf->element_size);
}

/* 
 * Pop the last element off the buffer.
 * If element_out is non-NULL, the popped element is copied there.
 * Returns 1 on success, or 0 if the buffer was empty.
 */
int buffer_pop(Buffer *buf, void *element_out) {
    if (buf->count == 0) {
        return 0; // Buffer is empty
    }
    buf->count--;
    void *source = (char*)buf->data + (buf->count * buf->element_size);
    if (element_out) {
        memcpy(element_out, source, buf->element_size);
    }
    return 1;
}

/* 
 * Clear the buffer by setting count to 0 and zeroing out the memory.
 */
void buffer_clear(Buffer *buf) {
    if (buf->data) {
        memset(buf->data, 0, buf->capacity * buf->element_size);
    }
    buf->count = 0;
}


ReturnStatus read_markers(const char* input_string, Buffer* output_buffer) {
    if (!input_string || !output_buffer)
        return RETURN_STATUS_VALUE_ERROR;
    
    /* Build a dispatch table using the X-macro.
       The table is ordered so that longer tokens come before shorter ones.
       Now we also add entries for '[' and ']' to be parsed as lparen and rparen.
    */
    static const struct {
        const char *token;
        size_t len;
        MarkerType type;
    } dispatch_table[] = {
        #define X(TYPE, PRINT_REPR, LEN) { PRINT_REPR, LEN, TYPE },
        MARKER_TOKENS(X)
        #undef X
        { "[", 1, MARKER_LPAREN },
        { "]", 1, MARKER_RPAREN }
    };
    const size_t num_tokens = sizeof(dispatch_table) / sizeof(dispatch_table[0]);
    
    size_t i = 0;
    while (input_string[i] != '\0') {
        /* Skip whitespace */
        while (input_string[i] != '\0' && isspace((unsigned char)input_string[i]))
            i++;
        if (input_string[i] == '\0')
            break;
        
        Marker marker;
        marker.bidx = i;
        int matched = 0;
        
        /* Check fixed tokens from the dispatch table */
        for (size_t t = 0; t < num_tokens; t++) {
            size_t token_len = dispatch_table[t].len; // Hardcoded length.
            if (strncmp(input_string + i, dispatch_table[t].token, token_len) == 0) {
                marker.type = dispatch_table[t].type;
                marker.eidx = i + token_len;
                if (!buffer_push(output_buffer, &marker))
                    return RETURN_STATUS_RUNTIME_ERROR;
                i += token_len;
                matched = 1;
                break;
            }
        }
        if (matched)
            continue;
        
        /* Handle string literals */
        if (input_string[i] == '"') {
            marker.bidx = i;
            i++; // Skip opening quote.
            while (input_string[i] != '\0' && input_string[i] != '"') {
                if (input_string[i] == '\\') {
                    i++; // Skip escape character.
                    if (input_string[i] != '\0')
                        i++;
                    else
                        break;
                } else {
                    i++;
                }
            }
            if (input_string[i] == '"') {
                i++; // Include closing quote.
            } else {
                return RETURN_STATUS_VALUE_ERROR; // Unclosed string literal.
            }
            marker.type = MARKER_STRING;
            marker.eidx = i;
            if (!buffer_push(output_buffer, &marker))
                return RETURN_STATUS_RUNTIME_ERROR;
            continue;
        }
        
        /* Parse a generic token (which could be an int, float, or symbol).
           Now, we also stop if we encounter a left or right parenthesis or bracket.
        */
        size_t start = i;
        while (input_string[i] != '\0' &&
               !isspace((unsigned char)input_string[i]) &&
               input_string[i] != '"' &&
               input_string[i] != '(' &&
               input_string[i] != ')' &&
               input_string[i] != '[' &&
               input_string[i] != ']') {
            i++;
        }
        marker.bidx = start;
        marker.eidx = i;
        
        /* Classify the token as INT, FLOAT, or SYMBOL. */
        int isInt = 1;
        int isFloat = 0;
        size_t j = start;
        if (j < i && (input_string[j] == '+' || input_string[j] == '-'))
            j++;
        if (j == i) {
            marker.type = MARKER_SYMBOL;
        } else {
            for (; j < i; j++) {
                if (input_string[j] == '.') {
                    if (isFloat) { // Multiple dots -> not a valid number.
                        isInt = 0;
                        break;
                    }
                    isFloat = 1;
                } else if (!isdigit((unsigned char)input_string[j])) {
                    isInt = 0;
                    break;
                }
            }
            if (isInt && !isFloat)
                marker.type = MARKER_INT;
            else if (isFloat)
                marker.type = MARKER_FLOAT;
            else
                marker.type = MARKER_SYMBOL;
        }
        if (!buffer_push(output_buffer, &marker))
            return RETURN_STATUS_RUNTIME_ERROR;
    }
    
    return RETURN_STATUS_SUCCESS;
}


/*
 * Helper function to convert a MarkerType into a readable string.
 */
const char *marker_type_to_string(MarkerType type) {
    switch(type) {
        #define X(TYPE, PRINT_REPR, LEN) case TYPE: return #TYPE;
        MARKER_TOKENS(X)
        #undef X
        case MARKER_SYMBOL: return "MARKER_SYMBOL";
        case MARKER_STRING: return "MARKER_STRING";
        case MARKER_INT:    return "MARKER_INT";
        case MARKER_FLOAT:  return "MARKER_FLOAT";
        default:            return "MARKER_UNKNOWN";
    }
}


/*
 * pretty_print_marker_buffer: Given a buffer of Markers and the original input string,
 * print each marker's details. For each marker, print the type, begin and end indices,
 * and the text (by slicing the input string using bidx and eidx).
 */
void pretty_print_markers(Buffer *marker_buffer, const char *input) {
    if (!marker_buffer || !input) return;
    
    for (size_t i = 0; i < marker_buffer->count; i++) {
        Marker *m = (Marker *)buffer_nth(marker_buffer, i);
        if (!m) continue;
        printf("Marker %zu: Type: %-7s, bidx: %zu, eidx: %zu, text: '",
               i, marker_type_to_string(m->type), m->bidx, m->eidx);
        // Use printf's precision specifier to print the exact substring.
        printf("%.*s", (int)(m->eidx - m->bidx), input + m->bidx);
        printf("'\n");
    }
}
