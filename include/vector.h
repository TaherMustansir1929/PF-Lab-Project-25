#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h> // Required for size_t

// --- Structure Definition ---

/**
 * @brief A generic dynamic array (vector).
 * * Stores elements of a fixed size in a contiguous, dynamically
 * allocated memory block.
 */
typedef struct {
    void* data;          // Pointer to the allocated memory block
    size_t size;         // Current number of elements stored
    size_t capacity;     // Total number of elements the block can hold
    size_t element_size; // Size of a single element in bytes
} Vector;


// --- Public Function Prototypes ---

/**
 * @brief Initializes a vector to hold elements of a specific size.
 * @param v Pointer to the Vector structure to initialize.
 * @param element_size The size in bytes of each element (e.g., sizeof(int)).
 */
void vector_init(Vector* v, size_t element_size);

/**
 * @brief Frees all memory allocated by the vector.
 * @param v Pointer to the vector to free.
 */
void vector_free(Vector* v);

/**
 * @brief Adds an element to the end of the vector.
 * * Reallocates memory if the current capacity is exceeded.
 * @param v Pointer to the vector.
 * @param element Pointer to the element data to be copied into the vector.
 */
void vector_push_back(Vector* v, const void* element);

/**
 * @brief Removes the last element from the vector.
 * * Does nothing if the vector is empty.
 * @param v Pointer to the vector.
 */
void vector_pop_back(Vector* v);

/**
 * @brief Retrieves a pointer to the element at a specific index.
 * * WARNING: The returned pointer is only valid until the next
 * operation that might reallocate the vector's memory (e.g., vector_push_back).
 * @param v Pointer to the vector.
 * @param index The index of the element to retrieve.
 * @return A void pointer to the element's data, or NULL if the index is out of bounds.
 */
void* vector_get(const Vector* v, size_t index);

/**
 * @brief Updates the element at a specific index.
 * @param v Pointer to the vector.
 * @param index The index of the element to update.
 * @param element Pointer to the new element data to be copied.
 * @return 0 on success, -1 if the index is out of bounds.
 */
int vector_set(Vector* v, size_t index, const void* element);

/**
 * @brief Returns the number of elements currently in the vector.
 * @param v Pointer to the vector.
 * @return The size of the vector.
 */
size_t vector_size(const Vector* v);

/**
 * @brief Returns the total number of elements the vector can currently hold.
 * @param v Pointer to the vector.
 * @return The capacity of the vector.
 */
size_t vector_capacity(const Vector* v);

/**
 * @brief Checks if the vector contains no elements.
 * @param v Pointer to the vector.
 * @return 1 (true) if empty, 0 (false) otherwise.
 */
int vector_is_empty(const Vector* v);

#endif // VECTOR_H
