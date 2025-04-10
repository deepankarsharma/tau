#include <stdlib.h>
#include <stdfil.h>
#include <stdio.h>
#include <string.h> // For memcpy
#include <stddef.h> // For size_t

// Define the Buffer structure
typedef struct {
    void* base;         // Pointer to the start of the allocated memory
    size_t size;        // Current number of elements stored
    size_t capacity;    // Maximum number of elements the allocated memory can hold
    size_t element_size; // Size of one element in bytes
} Buffer;

/**
 * @brief Creates and initializes a Buffer structure using existing memory.
 *
 * @param base A pointer to an existing memory block allocated by the caller.
 * The buffer will use this memory initially. NOTE: If buffer_push
 * reallocates, the buffer takes ownership, and the original 'base'
 * pointer might become invalid. Free the memory using buffer_destroy.
 * @param element_size The size (in bytes) of each element to be stored.
 * @param capacity The initial capacity (in number of elements) of the provided 'base' memory.
 * @return An initialized Buffer structure.
 */
Buffer buffer_create(void* base, size_t element_size, size_t capacity) {
    if (element_size == 0) {
        fprintf(stderr, "Error: Element size cannot be zero.\n");
        // Return an invalid buffer or handle error appropriately
        // For simplicity, we'll proceed but this is unsafe.
        // A better approach might be to return a buffer with base=NULL.
    }
     if (base == NULL && capacity > 0) {
        fprintf(stderr, "Warning: Initial base is NULL but capacity is non-zero.\n");
        // This is allowed if the intention is for the first push to allocate.
    }
     if (base != NULL && capacity == 0) {
        fprintf(stderr, "Warning: Initial base is provided but capacity is zero.\n");
        // This means the first push will immediately trigger reallocation.
    }

    Buffer buf;
    buf.base = base;
    buf.element_size = element_size;
    buf.capacity = capacity;
    buf.size = 0; // Initially, the buffer considers itself empty
    return buf;
}

/**
 * @brief Frees the memory allocated for the buffer's data.
 * Sets buffer fields to safe values.
 *
 * @param buf A pointer to the Buffer structure to destroy.
 */
void buffer_destroy(Buffer* buf) {
    if (buf && buf->base) {
        free(buf->base);
        buf->base = NULL;
        buf->size = 0;
        buf->capacity = 0;
        buf->element_size = 0; // Optional reset
    }
}


/**
 * @brief Adds an element to the end of the buffer, resizing if necessary.
 *
 * @param buf A pointer to the Buffer structure.
 * @param element A pointer to the element data to be added.
 * @return 0 on success, -1 on failure (e.g., memory allocation failed).
 */
int buffer_push(Buffer* buf, const void* element) {
    if (!buf || !element) {
        fprintf(stderr, "Error: Invalid arguments to buffer_push.\n");
        return -1;
    }
    if (buf->element_size == 0) {
         fprintf(stderr, "Error: Cannot push to buffer with zero element size.\n");
        return -1;
    }

    // Check if reallocation is needed
    if (buf->size == buf->capacity) {
        // Determine new capacity (common strategy: double, or start small)
        size_t new_capacity = (buf->capacity == 0) ? 8 : buf->capacity * 2;
        size_t new_size_bytes = new_capacity * buf->element_size;

        // Prevent overflow when calculating new size
        if (new_capacity < buf->capacity || new_size_bytes / new_capacity != buf->element_size) {
             fprintf(stderr, "Error: Buffer capacity overflow during resize calculation.\n");
             return -1;
        }


        // Reallocate memory. realloc handles NULL buf->base correctly (acts like malloc).
        void* new_base = realloc(buf->base, new_size_bytes);

        if (new_base == NULL) {
            // Handle realloc failure
            fprintf(stderr, "Error: Failed to reallocate buffer (requested %zu bytes).\n", new_size_bytes);
            // The original buf->base is still valid if realloc fails!
            return -1; // Indicate failure
        }

        // Update buffer fields ONLY on successful reallocation
        buf->base = new_base;
        buf->capacity = new_capacity;
    }

    // Calculate the destination address for the new element
    // Cast base to char* for byte-level pointer arithmetic
    char* dest = (char*)buf->base + (buf->size * buf->element_size);

    // Copy the element data into the buffer
    memcpy(dest, element, buf->element_size);

    // Increment the size
    buf->size++;

    return 0; // Indicate success
}

/**
 * @brief Gets a pointer to the nth element in the buffer.
 *
 * @param buf A pointer to the Buffer structure (const, as we don't modify it).
 * @param n The zero-based index of the element to retrieve.
 * @return A pointer to the nth element, or NULL if the index is out of bounds.
 * The caller must cast the returned void* to the correct element type.
 */
void* buffer_get_nth(const Buffer* buf, size_t n) {
    if (!buf || n >= buf->size) {
        // Index out of bounds or invalid buffer
        return NULL;
    }

    // Calculate address of the nth element
    // Cast base to char* for byte-level pointer arithmetic
    return (char*)buf->base + (n * buf->element_size);
}


enum NodeType { NODE_SYMBOL, NODE_STRING, NODE_LIST };

struct Annotation {
};

struct Type {
};

struct String {
};

struct Function {
};

struct Argument {
    Type type;
    Name name;
};

struct If {
};

struct Expression {
};

struct Cond {
};

int main(void) {
    return 0;
}
