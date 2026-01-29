/*!
    @file vector.h header file
    @brief  generic dynamic array (vector) implementation for C.

    This header provides a type-safe, efficient, and flexible vector (dynamic array) solution for C projects.
    Easily create dynamic arrays for any data type (primitives, structs, pointers) with automatic memory management,
    safe access, and modern iteration features.

    @author Semih "zyr1on" Özdemir

    @details
    - Type-generic: Instantiates vectors for any type via macros.
    - Dynamic: Grows and shrinks automatically as needed.
    - Safety: Provides bounds checking in debug mode.
    - Modern: Includes handy iteration and batch operations.

    @warning
    - Always initialize your vector with `vector_init()` before use.
    - Always destroy your vector with `vector_destroy()` after use to avoid memory leaks.
    - This header relies on C99 standard features (variadic macros, etc).
*/

#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define VECTOR_MAGIC_INIT 0xDEADBEEF
#define VECTOR_MAGIC_DESTROYED 0xFEEDFACE

typedef struct {
    void *data;
    size_t size;
    size_t capacity;
    uint32_t magic;
} VectorBase;

// Generic vector structure with magic number
#define VECTOR_DEFINE(type) \
    struct { \
        type *data; \
        size_t size; \
        size_t capacity; \
        uint32_t magic; \
    }

// vector type declaration macro
#define vector(type) VECTOR_DEFINE(type)

#define vector_is_valid(vec) ((vec).magic == VECTOR_MAGIC_INIT)

/** 
Initializes the vector structure. Includes a safety check against 
double-initialization to prevent memory leaks (orphaning the 'data' pointer)
if called on an already active vector.
* @param vec The vector structure to initialize (passed by value/reference, not pointer). 
*/
#define vector_init(vec) do { \
    if ((vec).magic == VECTOR_MAGIC_INIT) { \
        fprintf(stderr, "[!] Warning: Vector already initialized: 'vector_init' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    (vec).data = NULL; \
    (vec).size = 0; \
    (vec).capacity = 0; \
    (vec).magic = VECTOR_MAGIC_INIT; \
} while(0)

// growth strategy
#define VECTOR_GROW_CAPACITY(cap) ((cap) < 4 ? 4 : (cap) << 1)


/** Appends an element to the end of the vector, automatically resizing the 
internal buffer if the capacity is exceeded. Performs validation for 
initialization state and memory allocation success to ensure stability.
* @param vec The vector structure to modify (passed by value/reference).
* @param value The element to be added (must match the vector's type).
*/
#define vector_push_back(vec, value) do { \
    if (__builtin_expect((vec).magic != VECTOR_MAGIC_INIT, 0)) { \
        fprintf(stderr, "[x] Error: Vector not initialized before: 'vector_push_back' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (__builtin_expect((vec).size >= (vec).capacity, 0)) { \
        size_t new_capacity = VECTOR_GROW_CAPACITY((vec).capacity); \
        typeof((vec).data) new_data = realloc((vec).data, new_capacity * sizeof(*(vec).data)); \
        if (__builtin_expect(new_data != NULL, 1)) { \
            (vec).data = new_data; \
            (vec).capacity = new_capacity; \
        } else { \
            fprintf(stderr, "[x] Error: Memory allocation failed: 'vector_push_back': at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
    } \
    (vec).data[(vec).size++] = (value); \
} while(0)

/** Returns a pointer to the next available slot in the vector, resizing memory 
if necessary. Allows for zero-copy construction of elements directly 
into the vector's buffer.
* @param vec The vector structure to modify.
* @return A pointer to the newly reserved slot, or NULL on failure.
*/
#define private_vector_emplace_back_ptr(vec) ({ \
    typeof((vec).data) _slot_ptr = NULL; \
    int _success = 1; \
    if (__builtin_expect((vec).magic != VECTOR_MAGIC_INIT, 0)) { \
        fprintf(stderr, "[x] Error: Vector not initialized.\n"); \
        _success = 0; \
    } \
    else if (__builtin_expect((vec).size >= (vec).capacity, 0)) { \
        size_t _new_cap = VECTOR_GROW_CAPACITY((vec).capacity); \
        typeof((vec).data) _new_data = realloc((vec).data, _new_cap * sizeof(*(vec).data)); \
        if (__builtin_expect(_new_data != NULL, 1)) { \
            (vec).data = _new_data; \
            (vec).capacity = _new_cap; \
        } else { \
            fprintf(stderr, "[x] Error: Realloc failed.\n"); \
            _success = 0; \
        } \
    } \
    if (_success) { \
        _slot_ptr = &(vec).data[(vec).size++]; \
    } \
    _slot_ptr; /* return of macro (Pointer) */ \
})

/**
 * Constructs an object in-place at the end of the vector.
 * Uses C99 compound literals to cast the arguments to the vector's type.
 * usage: vector_emplace_back(vec, 10);
 * usage: vector_emplace_back(vec, .x=10, .y=20);
 */
#define vector_emplace_back(vec, ...) do { \
    typeof(*(vec).data) * _slot = private_vector_emplace_back_ptr(vec); \
    if (_slot) { \
        *_slot = (typeof(*(vec).data)){ __VA_ARGS__ }; \
    } \
} while(0)


#define vector_at(vec, index) \
    (((vec).magic == VECTOR_MAGIC_INIT && (index) < (vec).size) ? \
     (vec).data[index] : \
     (fprintf(stderr, "Error: Invalid vector access at %s:%d\n", __FILE__, __LINE__), abort(), (vec).data[0]))


#define vector_size(vec) ((vec).size)
#define vector_bytesize(vec) ((vec).size * sizeof(*(vec).data))
#define vector_capacity(vec) ((vec).capacity)
#define vector_empty(vec) ((vec).size == 0)
#define vector_back(vec) ((vec).data[(vec).size - 1])
#define vector_front(vec) ((vec).data[0])


#define vector_pop_back(vec) do { \
    if (__builtin_expect((vec).magic != VECTOR_MAGIC_INIT, 0)) { \
        fprintf(stderr, "[x] Error: Vector not initialized before 'vector_pop_back' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (__builtin_expect((vec).size == 0, 0)) { \
        fprintf(stderr, "[x] Error: Cannot 'vector_pop_back' from empty vector at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    (vec).size--; \
} while(0)

// clear ( set size to 0) basic!
#define vector_clear(vec) do { \
    if ((vec).magic != VECTOR_MAGIC_INIT) { \
        fprintf(stderr, "[x] Error: Vector not initialized before 'vector_pop_back' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    (vec).size = 0; \
} while(0)

/** Deallocates the vector's underlying memory buffer and resets its state. 
Implements safeguards against double-free errors and attempts to destroy 
uninitialized vectors to ensure heap integrity.
* @param vec The vector structure to destroy. 
*/
#define vector_destroy(vec) do { \
    if ((vec).magic == VECTOR_MAGIC_DESTROYED) { \
        fprintf(stderr, "[x] Error: Vector already destroyed at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((vec).magic != VECTOR_MAGIC_INIT) { \
        fprintf(stderr, "[x] Error: Cannot 'vector_destroy' uninitialized vector_destroy at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((vec).data) { \
        free((vec).data); \
        (vec).data = NULL; \
    } \
    (vec).size = 0; \
    (vec).capacity = 0; \
    (vec).magic = VECTOR_MAGIC_DESTROYED; \
} while(0)

// reserve with alignment
#define vector_reserve(vec, new_capacity) do { \
    if ((vec).magic != VECTOR_MAGIC_INIT) { \
        fprintf(stderr, "[x] Error: Vector not initialized before 'vector_reserve' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((new_capacity) <= (vec).capacity) break; \
    size_t aligned_capacity = (new_capacity); \
    if (aligned_capacity < 4) aligned_capacity = 4; \
    typeof((vec).data) new_data = realloc((vec).data, aligned_capacity * sizeof(*(vec).data)); \
    if (__builtin_expect(new_data != NULL, 1)) { \
        (vec).data = new_data; \
        (vec).capacity = aligned_capacity; \
    } else { \
        fprintf(stderr, "Error: Memory allocation failed in 'vector_reserve' at %s:%d\n", __FILE__, __LINE__); \
    } \
} while(0)

// // Bulk operations for better performance
// #define vector_push_back_bulk(vec, values, count) do { \
//     if ((vec).magic != VECTOR_MAGIC_INIT) { \
//         fprintf(stderr, "Error: Vector not initialized before push_back_bulk at %s:%d\n", __FILE__, __LINE__); \
//         break; \
//     } \
//     size_t needed_capacity = (vec).size + (count); \
//     if (needed_capacity > (vec).capacity) { \
//         size_t new_capacity = (vec).capacity; \
//         while (new_capacity < needed_capacity) { \
//             new_capacity = VECTOR_GROW_CAPACITY(new_capacity); \
//         } \
//         typeof((vec).data) new_data = realloc((vec).data, new_capacity * sizeof(*(vec).data)); \
//         if (__builtin_expect(new_data != NULL, 1)) { \
//             (vec).data = new_data; \
//             (vec).capacity = new_capacity; \
//         } else { \
//             fprintf(stderr, "Error: Memory allocation failed in push_back_bulk at %s:%d\n", __FILE__, __LINE__); \
//             break; \
//         } \
//     } \
//     memcpy((vec).data + (vec).size, (values), (count) * sizeof(*(vec).data)); \
//     (vec).size += (count); \
// } while(0)


/** Resizes the vector to contain 'new_size' elements. If the vector is expanded, 
new slots are populated with 'def_val'. If reduced, the vector is truncated. 
Automatically handles capacity reservation if the new size exceeds current capacity.
* @param vec The vector structure to modify.
* @param new_size The new size of the vector.
* @param def_val The value to initialize new elements with (if expanding).
*/
#define vector_resize(vec, new_size, def_val) do { \
    if ((vec).magic != VECTOR_MAGIC_INIT) { \
        fprintf(stderr, "[x] Error: Vector not initialized before 'vector_resize' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((new_size) > (vec).capacity) { \
        vector_reserve(vec, new_size); \
    } \
    if ((new_size) > (vec).size) { \
        for (size_t i = (vec).size; i < (new_size); i++) { \
            (vec).data[i] = (def_val); \
        } \
    } \
    (vec).size = (new_size); \
} while(0)

/** Requests the removal of unused capacity to reduce memory usage. 
If the vector is empty, the underlying buffer is fully deallocated. 
Otherwise, reallocates the buffer to match the current size exactly.
* @param vec The vector structure to fit. 
*/
#define vector_shrink_to_fit(vec) do { \
    if ((vec).magic != VECTOR_MAGIC_INIT) { \
        fprintf(stderr, "[x] Error: Vector not initialized before 'vector_shrink_to_fit' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((vec).size == (vec).capacity) break; \
    if ((vec).size == 0) { \
        free((vec).data); \
        (vec).data = NULL; \
        (vec).capacity = 0; \
    } else { \
        typeof((vec).data) new_data = realloc((vec).data, (vec).size * sizeof(*(vec).data)); \
        if (new_data) { \
            (vec).data = new_data; \
            (vec).capacity = (vec).size; \
        } \
    } \
} while(0)

/** Iterates over each element in the vector using a pointer. 
* @note **Warning:** Modifying the vector's size (e.g., push_back/resize) inside 
* this loop may invalidate the iterator pointer due to potential memory reallocation.
* @param vec The vector to iterate over.
* @param item The variable name to be used for the iterator pointer.
*/
#define vector_foreach(vec, item) \
    for (typeof(*(vec).data) *item = (vec).data; \
         item < (vec).data + (vec).size; \
         ++item)

/** Searches for the first occurrence of 'value' using a custom comparison predicate. 
* Optimized with 4-step loop unrolling to reduce branch overhead and improve 
* instruction pipelining.
* @param vec The vector to search.
* @param value The target value to find.
* @param cmp_func A comparison function or macro, called as cmp_func(item, value).
* @return The index of the first match, or -1 if not found.
*/
#define vector_find_custom(vec, value, cmp_func) ({ \
    int _result = -1; \
    if ((vec).magic != VECTOR_MAGIC_INIT) { \
        fprintf(stderr, "[x] Error: Vector not initialized before 'vector_find_custom_fast'\n"); \
    } else { \
        typeof(*(vec).data) _search_value = (value); \
        size_t _size = (vec).size; \
        size_t i = 0; \
        for (; i + 3 < _size; i += 4) { \
            if (cmp_func((vec).data[i], _search_value)) { \
                _result = (int)i; \
                break; \
            } \
            if (cmp_func((vec).data[i+1], _search_value)) { \
                _result = (int)(i+1); \
                break; \
            } \
            if (cmp_func((vec).data[i+2], _search_value)) { \
                _result = (int)(i+2); \
                break; \
            } \
            if (cmp_func((vec).data[i+3], _search_value)) { \
                _result = (int)(i+3); \
                break; \
            } \
        } \
        \
        /* Handle remaining elements */ \
        if (_result == -1) { \
            for (; i < _size; ++i) { \
                if (cmp_func((vec).data[i], _search_value)) { \
                    _result = (int)i; \
                    break; \
                } \
            } \
        } \
    } \
    _result; \
})

/** Searches for the first occurrence of 'value' using the standard equality operator (==). 
* Optimized with 4-step loop unrolling.
* @note **Type Restriction:** Valid ONLY for scalar types (integers, floats, pointers, enums). 
* **DO NOT** use with structs or for string content comparison (use 'vector_find_custom' instead).
* @param vec The vector to search.
* @param value The value to compare against.
* @return The index of the first match, or -1 if not found.
*/
#define vector_find(vec, value) ({ \
    int _result = -1; \
    if ((vec).magic != VECTOR_MAGIC_INIT) \
        fprintf(stderr, "[x] Error: Vector not initialized before 'vector_find' at %s:%d\n", __FILE__, __LINE__); \
    else { \
        typeof(*(vec).data) _search_value = (value); \
        size_t i = 0; \
        size_t size = (vec).size; \
        for (; i + 3 < size; i += 4) { \
            if ((vec).data[i] == _search_value) { _result = i; break; } \
            if ((vec).data[i+1] == _search_value) { _result = i+1; break; } \
            if ((vec).data[i+2] == _search_value) { _result = i+2; break; } \
            if ((vec).data[i+3] == _search_value) { _result = i+3; break; } \
        } \
        if (_result == -1) { \
            for (; i < size; ++i) { \
                if ((vec).data[i] == _search_value) { \
                    _result = i; \
                    break; \
                } \
            } \
        } \
    } \
    _result; \
})

// !! PRIVATE !! 
static inline int private_vector_push_back_args_inline(void *vec_ptr, size_t element_size, 
                                               const void *elements, size_t count) {
    //struct { void *data; size_t size; size_t capacity; uint32_t magic; } *vec = vec_ptr;
    VectorBase* vec = (VectorBase*) vec_ptr;
	if (__builtin_expect(vec->magic != VECTOR_MAGIC_INIT, 0))
        return -1; // Error
    size_t new_size = vec->size + count;
    if (__builtin_expect(new_size > vec->capacity, 0)) {
        size_t new_capacity = vec->capacity;
        if (__builtin_expect(new_capacity == 0, 0))
            new_capacity = count > 4 ? count : 4;
        else
            while (new_capacity < new_size)
                new_capacity = new_capacity * 2;
        void *new_data = realloc(vec->data, new_capacity * element_size);
        if (__builtin_expect(new_data != NULL, 1)) {
            vec->data = new_data;
            vec->capacity = new_capacity;
        } 
        else return -1; // Error
    }
    memcpy((char*)vec->data + vec->size * element_size, elements, count * element_size);
    vec->size = new_size;
    return 0;
}

/** Appends multiple elements to the vector in a single operation.
* Utilizes a temporary stack array to hold variadic arguments before performing 
* a bulk insertion.
* @note **Performance:** efficient for batch updates (single resize check), 
* but incurs stack memory usage proportional to the number of arguments.
* @param vec The vector structure to modify.
* @param ... Comma-separated list of values to append (must match vector type).
*/
#define vector_push_back_args(vec, ...) do { \
    typeof(*(vec).data) tmp[] = {__VA_ARGS__}; \
    if (private_vector_push_back_args_inline(&(vec), sizeof(*(vec).data), tmp, \
                                     sizeof(tmp) / sizeof(tmp[0])) != 0) { \
        fprintf(stderr, "[x] Error: vector_push_back_args failed at %s:%d\n (maybe not initialized)", __FILE__, __LINE__); \
    } \
} while(0)

/** Inserts 'value' at the specified index, shifting subsequent elements to the right 
* to create space. Automatically handles memory resizing if capacity is exceeded.
* @note **Performance:** Linear complexity O(N) due to memory shifting (memmove). 
* Use sparingly on large vectors.
* @note **Safety:** Validates that 'position' is within bounds (<= size).
* @param vec The vector structure to modify.
* @param position The index at which to insert (0 to size).
* @param value The element to insert.
*/
#define vector_insert(vec, position, value) do { \
    if (__builtin_expect((vec).magic != VECTOR_MAGIC_INIT, 0)) { \
        fprintf(stderr, "[x] Error: Vector not initialized before: 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    size_t pos = (position); \
    if (__builtin_expect(pos > (vec).size, 0)) { \
        fprintf(stderr, "[x] Error: Insert position out of bounds: 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (__builtin_expect((vec).size >= (vec).capacity, 0)) { \
        size_t new_capacity = VECTOR_GROW_CAPACITY((vec).capacity); \
        typeof((vec).data) new_data = realloc((vec).data, new_capacity * sizeof(*(vec).data)); \
        if (__builtin_expect(new_data != NULL, 1)) { \
            (vec).data = new_data; \
            (vec).capacity = new_capacity; \
        } else { \
            fprintf(stderr, "[x] Error: Memory allocation failed: 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
    } \
    if (pos < (vec).size) { \
        memmove(&(vec).data[pos + 1], &(vec).data[pos], ((vec).size - pos) * sizeof(*(vec).data)); \
    } \
    (vec).data[pos] = (value); \
    (vec).size++; \
} while(0)


#define vector_insert_range(vec, pos, arr, count) do { \
    if (__builtin_expect((vec).magic != VECTOR_MAGIC_INIT, 0)) { \
        fprintf(stderr, "[x] Error: Vector not initialized before: 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (__builtin_expect((arr) == NULL, 0)) { \
        fprintf(stderr, "[x] Error: Source array is NULL: 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    size_t _pos = (pos); \
    size_t insert_count = (count); \
    if (__builtin_expect(_pos > (vec).size, 0)) { \
        fprintf(stderr, "[x] Error: Insert position out of bounds: 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (__builtin_expect(insert_count == 0, 0)) { \
        break; \
    } \
    size_t new_size = (vec).size + insert_count; \
    if (new_size > (vec).capacity) { \
        size_t new_capacity = new_size; \
        if (new_capacity < VECTOR_GROW_CAPACITY((vec).capacity)) { \
            new_capacity = VECTOR_GROW_CAPACITY((vec).capacity); \
        } \
        typeof((vec).data) new_data = realloc((vec).data, new_capacity * sizeof(*(vec).data)); \
        if (__builtin_expect(new_data != NULL, 1)) { \
            (vec).data = new_data; \
            (vec).capacity = new_capacity; \
        } else { \
            fprintf(stderr, "[x] Error: Memory allocation failed: 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
    } \
    if (_pos < (vec).size) { \
        memmove(&(vec).data[_pos + insert_count], &(vec).data[_pos], ((vec).size - _pos) * sizeof(*(vec).data)); \
    } \
    memcpy(&(vec).data[_pos], (arr), insert_count * sizeof(*(vec).data)); \
    (vec).size = new_size; \
} while(0)

// !! PRIVATE !! 
static inline int private_vector_insert_args_inline(void *vec_ptr, size_t element_size, 
                                            size_t index,
                                            const void *elements, size_t count) {
    //struct { void *data; size_t size; size_t capacity; uint32_t magic; } *vec = vec_ptr;
	VectorBase* vec = (VectorBase*) vec_ptr;
    if (__builtin_expect(vec->magic != VECTOR_MAGIC_INIT, 0))
        return -1; // Error
    if (__builtin_expect(index > vec->size, 0))
        return -1; // Invalid index
    size_t new_size = vec->size + count;
    if (__builtin_expect(new_size > vec->capacity, 0)) {
        size_t new_capacity = vec->capacity;
        if (__builtin_expect(new_capacity == 0, 0))
            new_capacity = count > 4 ? count : 4;
        else
            while (new_capacity < new_size)
                new_capacity = new_capacity * 2;
        void *new_data = realloc(vec->data, new_capacity * element_size);
        if (__builtin_expect(new_data != NULL, 1)) {
            vec->data = new_data;
            vec->capacity = new_capacity;
        } 
        else return -1; // Error
    }
    memmove(
        (char*)vec->data + (index + count) * element_size,
        (char*)vec->data + index * element_size,
        (vec->size - index) * element_size
    );
    memcpy((char*)vec->data + index * element_size, elements, count * element_size);
    vec->size = new_size;
    return 0;
}

#define vector_insert_args(vec, idx, ...) do { \
    typeof(*(vec).data) tmp[] = {__VA_ARGS__}; \
    if (private_vector_insert_args_inline(&(vec), sizeof(*(vec).data), (idx), tmp, \
                                  sizeof(tmp) / sizeof(tmp[0])) != 0) { \
        fprintf(stderr, "[x] Error: vector_insert_args failed at %s:%d\n", __FILE__, __LINE__); \
    } \
} while(0)

// #define vector_insert_args(vec, pos, first, ...) do { \
//     typeof(first) temp_array[] = {first, __VA_ARGS__}; \
//     size_t temp_count = sizeof(temp_array)/sizeof(temp_array[0]); \
//     vector_insert_range(vec, pos, temp_array, temp_count); \
// } while(0)


/** Exchanges the contents of two vectors in constant time O(1) by swapping 
* their internal pointers and metadata. No deep copying is performed.
* @note **Safety:** Verifies initialization status and ensures both vectors 
* store elements of the same size (basic type safety) to prevent corruption.
* @param vec1 The first vector.
* @param vec2 The second vector.
*/
#define vector_swap(vec1, vec2) do { \
    if (__builtin_expect((vec1).magic != VECTOR_MAGIC_INIT, 0)) { \
        fprintf(stderr, "[x] Error: First vector not initialized before: 'vector_swap' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (__builtin_expect((vec2).magic != VECTOR_MAGIC_INIT, 0)) { \
        fprintf(stderr, "[x] Error: Second vector not initialized before: 'vector_swap' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    /* Type safety check - both vectors must be of same type */ \
    if (__builtin_expect(sizeof(*(vec1).data) != sizeof(*(vec2).data), 0)) { \
        fprintf(stderr, "[x] Error: Vector type mismatch in 'vector_swap' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    /* Swap data pointers */ \
    typeof((vec1).data) temp_data = (vec1).data; \
    (vec1).data = (vec2).data; \
    (vec2).data = temp_data; \
    /* Swap sizes */ \
    size_t temp_size = (vec1).size; \
    (vec1).size = (vec2).size; \
    (vec2).size = temp_size; \
    /* Swap capacities */ \
    size_t temp_capacity = (vec1).capacity; \
    (vec1).capacity = (vec2).capacity; \
    (vec2).capacity = temp_capacity; \
    /* Magic numbers remain the same - both should be VECTOR_MAGIC_INIT */ \
} while(0)

/*!    
@note: OLD VERSION WITHOUT inline
#define vector_push_back_args(vec, ...) do { \
    if (__builtin_expect((vec).magic != VECTOR_MAGIC_INIT, 0)) { \
        fprintf(stderr, "[x] Error: Vector not initialized before: 'vector_push_back' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (__builtin_expect((vec).size >= (vec).capacity, 0)) { \
        size_t new_capacity = VECTOR_GROW_CAPACITY((vec).capacity); \
        typeof((vec).data) new_data = realloc((vec).data, new_capacity * sizeof(*(vec).data)); \
        if (__builtin_expect(new_data != NULL, 1)) { \
            (vec).data = new_data; \
            (vec).capacity = new_capacity; \
        } else { \
            fprintf(stderr, "[x] Error: Memory allocation failed: 'vector_push_back': at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
    } \
    typeof(*(vec).data) tmp[] = {__VA_ARGS__}; \
    size_t count = sizeof(tmp) / sizeof(tmp[0]); \
    for (size_t i = 0; i < count; ++i) { \
        vector_push_back(vec, tmp[i]); \
    } \
} while(0)
*/

#endif
