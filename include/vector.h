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

/**
 * @section Platform Abstraction Layer
 * -------------------------------------------------------------------------
 * This section provides a bridge between C (C99+) and Modern C++ compilers.
 * It handles differences in type deduction, namespace management, and 
 * strict type-checking rules.
 */

#ifdef __cplusplus
/* C++ Compiler Configuration */
    #include <cstdio>
    #include <cstdlib>
    #include <cstdint>
    #include <cstring>
    #include <type_traits>
    /**
    * @brief C++ Namespace and Type Deduction Macros
    * C++ `decltype(*ptr)` returns a reference (T&), which cannot be used for 
    * array declarations. TYPE_OF_VAL cleans the reference to get the raw type.
    */
    #define CLIB_PREFIX std::
    #define TYPE_OF(x) decltype(x)
    #define TYPE_OF_VAL(x) std::remove_reference<decltype(x)>::type
    #define NULL_PTR nullptr
#else 
/* Standard C Compiler Configuration */
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>
    /**
    * @brief C99 typeof Support
    * In C, typeof handles both values and pointers without reference issues.
    */
    #define CLIB_PREFIX 
    #define TYPE_OF(x) typeof(x)
    #define TYPE_OF_VAL(x) typeof(x)	
    #define NULL_PTR NULL
#endif

/**
 * @section Compiler Portability
 * -------------------------------------------------------------------------
 * __builtin_expect is GCC/Clang specific and not available on MSVC.
 * LIKELY/UNLIKELY macros provide a portable abstraction.
 */
#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

/**
 * @section Compile-time Type Safety
 * -------------------------------------------------------------------------
 * realloc is only safe for trivially copyable types (no constructor/destructor).
 * In C++ mode, this is enforced at compile time via static_assert.
 * In C mode, all types are implicitly trivially copyable — no check needed.
 *
 * If you hit this error, your struct has a non-trivial constructor/destructor.
 * Use plain C structs (POD types) with this vector implementation.
 */
#ifdef __cplusplus
    #define VECTOR_ASSERT_TRIVIAL(vec) \
        static_assert( \
            std::is_trivially_copyable<std::remove_pointer<TYPE_OF((vec).data)>::type>::value, \
            "vector<T>: T must be trivially copyable — realloc is unsafe for types with constructors/destructors" \
        )
#else
    #define VECTOR_ASSERT_TRIVIAL(vec) /* no-op in C */
#endif

/**
* @section Vector Core Metadata
* -------------------------------------------------------------------------
* Magic numbers used for initialization tracking and memory safety.
*/
 
#define VECTOR_MAGIC_INIT      0xDEADBEEF
#define VECTOR_MAGIC_DESTROYED 0xFEEDFACE

typedef struct {
    void    *data;
    size_t   size;
    size_t   capacity;
    uint32_t magic;
} VectorBase;

/* Generic vector structure with magic number */
#define VECTOR_DEFINE(type) \
    struct { \
        type    *data; \
        size_t   size; \
        size_t   capacity; \
        uint32_t magic; \
    }

/* vector type declaration macro */
#define vector(type) VECTOR_DEFINE(type)

#define vector_is_valid(vec) ((vec).magic == VECTOR_MAGIC_INIT)

/** 
 * Initializes the vector structure. Includes a safety check against 
 * double-initialization to prevent memory leaks (orphaning the 'data' pointer)
 * if called on an already active vector.
 * @param vec The vector structure to initialize.
 */
#define vector_init(vec) do { \
    VECTOR_ASSERT_TRIVIAL(vec); \
    if ((vec).magic == VECTOR_MAGIC_INIT) { \
        CLIB_PREFIX fprintf(stderr, "[!] Warning: Vector already initialized: 'vector_init' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    (vec).data     = NULL_PTR; \
    (vec).size     = 0; \
    (vec).capacity = 0; \
    (vec).magic    = VECTOR_MAGIC_INIT; \
} while(0)

/* Growth strategy: double capacity, minimum 4 */
#define VECTOR_GROW_CAPACITY(cap) ((cap) < 4 ? 4 : (cap) << 1)

/** 
 * Appends an element to the end of the vector, automatically resizing the 
 * internal buffer if the capacity is exceeded.
 * @param vec   The vector structure to modify.
 * @param value The element to be added (must match the vector's type).
 */
#define vector_push_back(vec, value) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before: 'vector_push_back' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((vec).size >= (vec).capacity)) { \
        size_t _new_cap = VECTOR_GROW_CAPACITY((vec).capacity); \
        TYPE_OF((vec).data) _new_data = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _new_cap * sizeof(*(vec).data)); \
        if (LIKELY(_new_data != NULL_PTR)) { \
            (vec).data     = _new_data; \
            (vec).capacity = _new_cap; \
        } else { \
            CLIB_PREFIX fprintf(stderr, "[x] Error: Memory allocation failed: 'vector_push_back' at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
    } \
    (vec).data[(vec).size++] = (value); \
} while(0)

/** 
 * Returns a pointer to the next available slot in the vector, resizing memory 
 * if necessary. Allows for zero-copy construction of elements directly 
 * into the vector's buffer.
 * @param vec The vector structure to modify.
 * @return    A pointer to the newly reserved slot, or NULL_PTR on failure.
 *
 * @note Uses GNU statement expression ({...}) — supported by GCC and Clang.
 *       Not supported on MSVC. If MSVC support is needed, convert to inline function.
 */
#define private_vector_emplace_back_ptr(vec) ({ \
    TYPE_OF_VAL((vec).data) _slot_ptr = NULL_PTR; \
    int _success = 1; \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized: 'vector_emplace_back' at %s:%d\n", __FILE__, __LINE__); \
        _success = 0; \
    } \
    else if (UNLIKELY((vec).size >= (vec).capacity)) { \
        size_t _new_cap = VECTOR_GROW_CAPACITY((vec).capacity); \
        TYPE_OF((vec).data) _new_data = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _new_cap * sizeof(*(vec).data)); \
        if (LIKELY(_new_data != NULL_PTR)) { \
            (vec).data     = _new_data; \
            (vec).capacity = _new_cap; \
        } else { \
            CLIB_PREFIX fprintf(stderr, "[x] Error: Realloc failed: 'vector_emplace_back' at %s:%d\n", __FILE__, __LINE__); \
            _success = 0; \
        } \
    } \
    if (_success) { \
        _slot_ptr = &(vec).data[(vec).size++]; \
    } \
    _slot_ptr; \
})

/**
 * Constructs an object in-place at the end of the vector.
 *
 * @note C99/C11: Supports designated initializers in any order:
 *       vector_emplace_back(vec, .y = 1.0f, .x = 2.0f);
 *
 * @note C++20: Designated initializers are supported but MUST follow
 *       struct field declaration order. Out-of-order will cause a compile error.
 *       vector_emplace_back(vec, .x = 2.0f, .y = 1.0f);  // OK if x declared before y
 *       vector_emplace_back(vec, .y = 1.0f, .x = 2.0f);  // compile ERROR in C++
 *
 * @note C++ before C++20: Designated initializers are NOT supported at all.
 *       Use positional initialization: vector_emplace_back(vec, 2.0f, 1.0f);
 *
 * Replaces C compound literal (Type){...} with portable `Type _tmp = {...}` 
 * which works in both C and C++.
 */
#define vector_emplace_back(vec, ...) do { \
    TYPE_OF_VAL(*(vec).data)* _slot = private_vector_emplace_back_ptr(vec); \
    if (_slot) { \
        TYPE_OF_VAL(*(vec).data) _tmp = { __VA_ARGS__ }; \
        *_slot = _tmp; \
    } \
} while(0)

/**
 * Safe element access with bounds checking.
 * Aborts on invalid access (out-of-bounds or uninitialized vector).
 */
#define vector_at(vec, index) \
    (((vec).magic == VECTOR_MAGIC_INIT && (index) < (vec).size) ? \
     (vec).data[index] : \
     (CLIB_PREFIX fprintf(stderr, "[x] Error: Invalid vector access at %s:%d\n", __FILE__, __LINE__), abort(), (vec).data[0]))

#define vector_size(vec)       ((vec).size)
#define vector_bytesize(vec)   ((vec).size * sizeof(*(vec).data))
#define vector_capacity(vec)   ((vec).capacity)
#define vector_empty(vec)      ((vec).size == 0)
#define vector_back(vec)       ((vec).data[(vec).size - 1])
#define vector_front(vec)      ((vec).data[0])

/**
 * Removes the last element from the vector.
 */
#define vector_pop_back(vec) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_pop_back' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((vec).size == 0)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Cannot 'vector_pop_back' from empty vector at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    (vec).size--; \
} while(0)

/**
 * Resets vector size to 0 without freeing memory.
 */
#define vector_clear(vec) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_clear' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    (vec).size = 0; \
} while(0)

/** 
 * Deallocates the vector's underlying memory buffer and resets its state. 
 * Implements safeguards against double-free and uninitialized destroy.
 * @param vec The vector structure to destroy. 
 */
#define vector_destroy(vec) do { \
    if ((vec).magic == VECTOR_MAGIC_DESTROYED) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector already destroyed at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((vec).magic != VECTOR_MAGIC_INIT) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Cannot destroy uninitialized vector at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((vec).data) { \
        free((vec).data); \
        (vec).data = NULL_PTR; \
    } \
    (vec).size     = 0; \
    (vec).capacity = 0; \
    (vec).magic    = VECTOR_MAGIC_DESTROYED; \
} while(0)

/** 
 * Reserves capacity for at least new_capacity elements.
 * No-op if current capacity already satisfies the request.
 *
 * @note FIX: Previously used new_capacity in realloc but aligned_capacity
 *       in the assignment — these are now consistent.
 */
#define vector_reserve(vec, new_capacity) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_reserve' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((new_capacity) <= (vec).capacity) break; \
    size_t _aligned_cap = (new_capacity) < 4 ? 4 : (new_capacity); \
    TYPE_OF((vec).data) _new_data = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _aligned_cap * sizeof(*(vec).data)); \
    if (LIKELY(_new_data != NULL_PTR)) { \
        (vec).data     = _new_data; \
        (vec).capacity = _aligned_cap; \
    } else { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Memory allocation failed in 'vector_reserve' at %s:%d\n", __FILE__, __LINE__); \
    } \
} while(0)

/** 
 * Resizes the vector to contain new_size elements.
 * Expands with def_val or truncates as needed.
 * @param vec      The vector structure to modify.
 * @param new_size The new size of the vector.
 * @param def_val  Value to fill new slots with (if expanding).
 */
#define vector_resize(vec, new_size, def_val) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_resize' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((new_size) > (vec).capacity) { \
        vector_reserve(vec, new_size); \
    } \
    if ((new_size) > (vec).size) { \
        TYPE_OF_VAL(*(vec).data) _fill_val = (def_val); \
        for (size_t _i = (vec).size; _i < (new_size); _i++) { \
            (vec).data[_i] = _fill_val; \
        } \
    } \
    (vec).size = (new_size); \
} while(0)

/** 
 * Shrinks the internal buffer to match the current size exactly.
 * If the vector is empty, fully deallocates the buffer.
 */
#define vector_shrink_to_fit(vec) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_shrink_to_fit' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((vec).size == (vec).capacity) break; \
    if ((vec).size == 0) { \
        free((vec).data); \
        (vec).data     = NULL_PTR; \
        (vec).capacity = 0; \
    } else { \
        TYPE_OF((vec).data) _new_data = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, (vec).size * sizeof(*(vec).data)); \
        if (LIKELY(_new_data != NULL_PTR)) { \
            (vec).data     = _new_data; \
            (vec).capacity = (vec).size; \
        } \
    } \
} while(0)

/** 
 * Iterates over each element in the vector using a pointer.
 * @warning Do NOT modify vector size (push/resize) inside this loop.
 * @param vec  The vector to iterate over.
 * @param item Iterator pointer variable name.
 */
#define vector_foreach(vec, item) \
    for (TYPE_OF_VAL(*(vec).data) *item = (vec).data; \
         item < (vec).data + (vec).size; \
         ++item)

/** 
 * Searches using a custom comparison predicate.
 * Optimized with 4-step loop unrolling.
 * @param vec      The vector to search.
 * @param value    The target value to find.
 * @param cmp_func Comparison function: cmp_func(item, value) -> nonzero if match.
 * @return Index of first match, or -1 if not found.
 *
 * @note Uses GNU statement expression ({...}) — GCC/Clang only.
 */
#define vector_find_custom(vec, value, cmp_func) ({ \
    int _result = -1; \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_find_custom' at %s:%d\n", __FILE__, __LINE__); \
    } else { \
        TYPE_OF_VAL(*(vec).data) _search_val = (value); \
        size_t _sz = (vec).size; \
        size_t _i  = 0; \
        for (; _i + 3 < _sz; _i += 4) { \
            if (cmp_func((vec).data[_i],   _search_val)) { _result = (int)_i;   break; } \
            if (cmp_func((vec).data[_i+1], _search_val)) { _result = (int)(_i+1); break; } \
            if (cmp_func((vec).data[_i+2], _search_val)) { _result = (int)(_i+2); break; } \
            if (cmp_func((vec).data[_i+3], _search_val)) { _result = (int)(_i+3); break; } \
        } \
        if (_result == -1) { \
            for (; _i < _sz; ++_i) { \
                if (cmp_func((vec).data[_i], _search_val)) { \
                    _result = (int)_i; \
                    break; \
                } \
            } \
        } \
    } \
    _result; \
})

/** 
 * Searches using == operator. Optimized with 4-step loop unrolling.
 * @note Only valid for scalar types (int, float, pointer, enum).
 *       Do NOT use with structs or strings — use vector_find_custom instead.
 * @return Index of first match, or -1 if not found.
 *
 * @note Uses GNU statement expression ({...}) — GCC/Clang only.
 */
#define vector_find(vec, value) ({ \
    int _result = -1; \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_find' at %s:%d\n", __FILE__, __LINE__); \
    } else { \
        TYPE_OF_VAL(*(vec).data) _search_val = (value); \
        size_t _i  = 0; \
        size_t _sz = (vec).size; \
        for (; _i + 3 < _sz; _i += 4) { \
            if ((vec).data[_i]   == _search_val) { _result = (int)_i;   break; } \
            if ((vec).data[_i+1] == _search_val) { _result = (int)(_i+1); break; } \
            if ((vec).data[_i+2] == _search_val) { _result = (int)(_i+2); break; } \
            if ((vec).data[_i+3] == _search_val) { _result = (int)(_i+3); break; } \
        } \
        if (_result == -1) { \
            for (; _i < _sz; ++_i) { \
                if ((vec).data[_i] == _search_val) { \
                    _result = (int)_i; \
                    break; \
                } \
            } \
        } \
    } \
    _result; \
})

/* !! PRIVATE !! — Do not call directly */
static inline int private_vector_push_back_args_inline(void *vec_ptr, size_t element_size, 
                                                       const void *elements, size_t count) {
    VectorBase *vec = (VectorBase *)vec_ptr;
    if (UNLIKELY(vec->magic != VECTOR_MAGIC_INIT))
        return -1;
    size_t new_size = vec->size + count;
    if (UNLIKELY(new_size > vec->capacity)) {
        size_t new_capacity = vec->capacity;
        if (UNLIKELY(new_capacity == 0))
            new_capacity = count > 4 ? count : 4;
        else
            while (new_capacity < new_size)
                new_capacity <<= 1;
        void *new_data = CLIB_PREFIX realloc(vec->data, new_capacity * element_size);
        if (LIKELY(new_data != NULL_PTR)) {
            vec->data     = new_data;
            vec->capacity = new_capacity;
        } else {
            return -1;
        }
    }
    memcpy((char *)vec->data + vec->size * element_size, elements, count * element_size);
    vec->size = new_size;
    return 0;
}

/** 
 * Appends multiple elements to the vector in a single operation.
 * @note Efficient for batch updates: single resize check, bulk memcpy.
 * @param vec The vector structure to modify.
 * @param ... Comma-separated values matching the vector's element type.
 */
#define vector_push_back_args(vec, ...) do { \
    TYPE_OF_VAL(*(vec).data) _tmp[] = { __VA_ARGS__ }; \
    if (private_vector_push_back_args_inline(&(vec), sizeof(*(vec).data), _tmp, \
                                             sizeof(_tmp) / sizeof(_tmp[0])) != 0) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector_push_back_args failed at %s:%d (maybe not initialized)\n", __FILE__, __LINE__); \
    } \
} while(0)

/** 
 * Inserts value at position, shifting subsequent elements right.
 * @note O(N) complexity due to memmove. Use sparingly on large vectors.
 * @param vec      The vector to modify.
 * @param position Insert index (0 to size).
 * @param value    Element to insert.
 */
#define vector_insert(vec, position, value) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before: 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    size_t _pos = (position); \
    if (UNLIKELY(_pos > (vec).size)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Insert position out of bounds: 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((vec).size >= (vec).capacity)) { \
        size_t _new_cap = VECTOR_GROW_CAPACITY((vec).capacity); \
        TYPE_OF((vec).data) _new_data = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _new_cap * sizeof(*(vec).data)); \
        if (LIKELY(_new_data != NULL_PTR)) { \
            (vec).data     = _new_data; \
            (vec).capacity = _new_cap; \
        } else { \
            CLIB_PREFIX fprintf(stderr, "[x] Error: Memory allocation failed: 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
    } \
    if (_pos < (vec).size) { \
        memmove(&(vec).data[_pos + 1], &(vec).data[_pos], ((vec).size - _pos) * sizeof(*(vec).data)); \
    } \
    (vec).data[_pos] = (value); \
    (vec).size++; \
} while(0)

/**
 * Inserts a range of elements from an array at position pos.
 * @note O(N) due to memmove + memcpy.
 */
#define vector_insert_range(vec, pos, arr, count) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before: 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((arr) == NULL_PTR)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Source array is NULL: 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    size_t _pos          = (pos); \
    size_t _insert_count = (count); \
    if (UNLIKELY(_pos > (vec).size)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Insert position out of bounds: 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY(_insert_count == 0)) break; \
    size_t _new_size = (vec).size + _insert_count; \
    if (_new_size > (vec).capacity) { \
        size_t _new_cap = VECTOR_GROW_CAPACITY((vec).capacity); \
        if (_new_cap < _new_size) _new_cap = _new_size; \
        TYPE_OF((vec).data) _new_data = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _new_cap * sizeof(*(vec).data)); \
        if (LIKELY(_new_data != NULL_PTR)) { \
            (vec).data     = _new_data; \
            (vec).capacity = _new_cap; \
        } else { \
            CLIB_PREFIX fprintf(stderr, "[x] Error: Memory allocation failed: 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
    } \
    if (_pos < (vec).size) { \
        memmove(&(vec).data[_pos + _insert_count], &(vec).data[_pos], \
                ((vec).size - _pos) * sizeof(*(vec).data)); \
    } \
    memcpy(&(vec).data[_pos], (arr), _insert_count * sizeof(*(vec).data)); \
    (vec).size = _new_size; \
} while(0)

/* !! PRIVATE !! — Do not call directly */
static inline int private_vector_insert_args_inline(void *vec_ptr, size_t element_size,
                                                    size_t index,
                                                    const void *elements, size_t count) {
    VectorBase *vec = (VectorBase *)vec_ptr;
    if (UNLIKELY(vec->magic != VECTOR_MAGIC_INIT)) return -1;
    if (UNLIKELY(index > vec->size))               return -1;
    size_t new_size = vec->size + count;
    if (UNLIKELY(new_size > vec->capacity)) {
        size_t new_capacity = vec->capacity;
        if (UNLIKELY(new_capacity == 0))
            new_capacity = count > 4 ? count : 4;
        else
            while (new_capacity < new_size)
                new_capacity <<= 1;
        void *new_data = CLIB_PREFIX realloc(vec->data, new_capacity * element_size);
        if (LIKELY(new_data != NULL_PTR)) {
            vec->data     = new_data;
            vec->capacity = new_capacity;
        } else {
            return -1;
        }
    }
    memmove(
        (char *)vec->data + (index + count) * element_size,
        (char *)vec->data + index * element_size,
        (vec->size - index) * element_size
    );
    memcpy((char *)vec->data + index * element_size, elements, count * element_size);
    vec->size = new_size;
    return 0;
}

/**
 * Inserts multiple values at index using variadic arguments.
 */
#define vector_insert_args(vec, idx, ...) do { \
    TYPE_OF_VAL(*(vec).data) _tmp[] = { __VA_ARGS__ }; \
    if (private_vector_insert_args_inline(&(vec), sizeof(*(vec).data), (idx), _tmp, \
                                          sizeof(_tmp) / sizeof(_tmp[0])) != 0) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector_insert_args failed at %s:%d\n", __FILE__, __LINE__); \
    } \
} while(0)

/** 
 * Swaps contents of two vectors in O(1) — pointer and metadata swap, no deep copy.
 * @note Both vectors must store elements of the same type (size-checked at runtime).
 */
#define vector_swap(vec1, vec2) do { \
    if (UNLIKELY((vec1).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: First vector not initialized before: 'vector_swap' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((vec2).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Second vector not initialized before: 'vector_swap' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY(sizeof(*(vec1).data) != sizeof(*(vec2).data))) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector type size mismatch in 'vector_swap' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    TYPE_OF((vec1).data) _tmp_data     = (vec1).data; \
    (vec1).data     = (vec2).data; \
    (vec2).data     = _tmp_data; \
    size_t _tmp_size     = (vec1).size; \
    (vec1).size     = (vec2).size; \
    (vec2).size     = _tmp_size; \
    size_t _tmp_cap      = (vec1).capacity; \
    (vec1).capacity = (vec2).capacity; \
    (vec2).capacity = _tmp_cap; \
} while(0)

#endif /* VECTOR_H */
