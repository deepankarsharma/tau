#pragma once

#include <stddef.h>

typedef enum {
  MARKER_LPAREN,
  MARKER_RPAREN,
  MARKER_SYMBOL,
  MARKER_STRING,
  MARKER_INT,
  MARKER_FLOAT,
  MARKER_QUOTE,             // '
  MARKER_QUASI_QUOTE,       // `
  MARKER_UNQUOTE,           // , 
  MARKER_UNQUOTE_SPLICING,  // ,@
  MARKER_SYNTAX,            // #'
  MARKER_QUASI_SYNTAX,      // #`
  MARKER_UNSYNTAX,          // #,
  MARKER_UNSYNTAX_SPLICING, // #,@
  MARKER_TRUE,              // #t
  MARKER_FALSE,             // #f
  MARKER_NIL,               // nil
} MarkerType;

typedef struct {
  size_t bidx;
  size_t eidx;
  MarkerType type;
} Marker;


typedef enum {
  RETURN_STATUS_SUCCESS,
  RETURN_STATUS_RETRYABLE_ERROR,
  RETURN_STATUS_VALUE_ERROR,
  RETURN_STATUS_RUNTIME_ERROR,
  RETURN_STATUS_PERMANENT_ERROR
} ReturnStatus;

/*
  Buffer: Resizable buffer
*/
typedef struct {
    void   *data;         // Pointer to the buffer memory
    size_t element_size;  // Size in bytes of one element
    size_t capacity;      // Allocated number of elements
    size_t count;         // Number of elements currently stored
} Buffer;


/* Buffer functions */
Buffer* buffer_create(size_t element_size, size_t initial_capacity);
void buffer_destroy(Buffer *buf);
void buffer_cleanup(Buffer **buf);
int buffer_push(Buffer *buf, const void *element);
void* buffer_nth(Buffer *buf, size_t n);
int buffer_pop(Buffer *buf, void *element_out);
void buffer_clear(Buffer *buf);

/* Scheme functions */
ReturnStatus read_markers(const char* input_string, Buffer* output_buffer);
ReturnStatus eval_buffer(Buffer *marker_buffer, const char *input);
const char *marker_type_to_string(MarkerType type);
void pretty_print_markers(Buffer *marker_buffer, const char *input);
