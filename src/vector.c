#include "vector.h"
#include <stdlib.h> // For malloc, realloc, free
#include <string.h> // For memcpy
#include <stdio.h>  // For perror

// Define an initial capacity for new vectors
#define VECTOR_INITIAL_CAPACITY 4

// --- Private Helper Function ---

/**
 * @brief Resizes the vector's internal storage.
 * * This is a helper function to double the capacity when needed.
 * It handles the memory reallocation and updates the vector's capacity.
 * If reallocation fails, it prints an error and exits.
 * @param v Pointer to the vector to resize.
 */
static void vector_resize(Vector* v) {
    // A common strategy is to double the capacity
    size_t new_capacity = (v->capacity == 0) ? VECTOR_INITIAL_CAPACITY : v->capacity * 2;
    
    // Attempt to reallocate the memory block
    void* new_data = realloc(v->data, new_capacity * v->element_size);
    if (new_data == NULL) {
        perror("Failed to reallocate memory for vector");
        exit(EXIT_FAILURE);
    }
    
    // Update the vector's data pointer and capacity
    v->data = new_data;
    v->capacity = new_capacity;
}


// --- Public Function Implementations ---

void vector_init(Vector* v, size_t element_size) {
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
    v->element_size = element_size;
}

void vector_free(Vector* v) {
    // free() is safe to call on NULL pointers
    free(v->data);
    
    // Reset struct members to prevent use-after-free
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
    v->element_size = 0;
}

void vector_push_back(Vector* v, const void* element) {
    // Check if we need more space. If so, resize.
    if (v->size >= v->capacity) {
        vector_resize(v);
    }
    
    // Calculate the memory address for the new element.
    // This is like calculating `&data[size]`.
    char* destination = (char*)v->data + (v->size * v->element_size);
    
    // Copy the provided element's data into the vector's storage.
    memcpy(destination, element, v->element_size);
    
    // Increment the size to reflect the new element.
    v->size++;
}

void vector_pop_back(Vector* v) {
    // Simply decrement the size. The data remains but will be overwritten
    // by the next push. This is a fast and common approach.
    if (v->size > 0) {
        v->size--;
    }
}

void* vector_get(const Vector* v, size_t index) {
    // Perform a bounds check.
    if (index >= v->size) {
        return NULL; // Return NULL for out-of-bounds access
    }
    
    // Calculate the address of the element at the given index.
    return (char*)v->data + (index * v->element_size);
}

int vector_set(Vector* v, size_t index, const void* element) {
    // Perform a bounds check.
    if (index >= v->size) {
        return -1; // Indicate failure
    }

    // Get the destination address.
    char* destination = (char*)v->data + (index * v->element_size);

    // Copy the new data over the old data.
    memcpy(destination, element, v->element_size);
    
    return 0; // Indicate success
}

size_t vector_size(const Vector* v) {
    return v->size;
}

size_t vector_capacity(const Vector* v) {
    return v->capacity;
}

int vector_is_empty(const Vector* v) {
    return v->size == 0;
}
