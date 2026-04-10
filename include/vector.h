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
    #define LIKELY(x)               __builtin_expect(!!(x), 1)
    #define UNLIKELY(x)             __builtin_expect(!!(x), 0)
    #define VECTOR_UNREACHABLE()    __builtin_unreachable()
#elif defined(_MSC_VER)
    #define LIKELY(x)               (x)
    #define UNLIKELY(x)             (x)
    #define VECTOR_UNREACHABLE()    __assume(0)
#else
    #define LIKELY(x)               (x)
    #define UNLIKELY(x)             (x)
    #define VECTOR_UNREACHABLE()    ((void)0)
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
 * @brief Enforces identical element types between two vectors at compile time.
 * @note  In C, only element sizes are compared (same-size different types are not caught).
 */
#ifdef __cplusplus
    #define VECTOR_ASSERT_SAME_TYPE(v1, v2) \
        static_assert( \
            std::is_same< \
                typename std::remove_pointer<TYPE_OF((v1).data)>::type, \
                typename std::remove_pointer<TYPE_OF((v2).data)>::type \
            >::value, \
            "vector_swap: both vectors must store the same type" \
        )
#else
    #define VECTOR_ASSERT_SAME_TYPE(v1, v2) \
        _Static_assert( \
            sizeof(*(v1).data) == sizeof(*(v2).data), \
            "vector_swap: element size mismatch" \
        )
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

/**
 * @brief Declares an anonymous vector struct for the given type.
 * @note  Use `vector(type)` as the variable type.
 *
 * @example
 *   vector(int) nums;
 *   vector_init(nums);
 */
#define VECTOR_DEFINE(type) \
    struct { \
        type     *data; \
        size_t    size; \
        size_t    capacity; \
        uint32_t  magic; \
    }

#define vector(type)        VECTOR_DEFINE(type)
#define vector_is_valid(v)  ((v).magic == VECTOR_MAGIC_INIT)

/** 
 * Initializes the vector structure. Includes a safety check against 
 * double-initialization to prevent memory leaks (orphaning the 'data' pointer)
 * if called on an already active vector.
 * @param vec The vector structure to initialize.
 */
#define vector_init(vec) do { \
    VECTOR_ASSERT_TRIVIAL(vec); \
    if ((vec).magic == VECTOR_MAGIC_INIT) { \
        CLIB_PREFIX fprintf(stderr, "[!] Warning: vector already initialized at %s:%d\n", __FILE__, __LINE__); \
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
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before 'vector_push_back' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((vec).size >= (vec).capacity)) { \
        size_t _nc = VECTOR_GROW_CAPACITY((vec).capacity); \
        TYPE_OF((vec).data) _nd = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _nc * sizeof(*(vec).data)); \
        if (UNLIKELY(_nd == NULL_PTR)) { \
            CLIB_PREFIX fprintf(stderr, "[x] Error: allocation failed in 'vector_push_back' at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
        (vec).data     = _nd; \
        (vec).capacity = _nc; \
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
    TYPE_OF_VAL((vec).data) _slot = NULL_PTR; \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before 'vector_emplace_back' at %s:%d\n", __FILE__, __LINE__); \
    } else { \
        if (UNLIKELY((vec).size >= (vec).capacity)) { \
            size_t _nc = VECTOR_GROW_CAPACITY((vec).capacity); \
            TYPE_OF((vec).data) _nd = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _nc * sizeof(*(vec).data)); \
            if (LIKELY(_nd != NULL_PTR)) { \
                (vec).data     = _nd; \
                (vec).capacity = _nc; \
            } else { \
                CLIB_PREFIX fprintf(stderr, "[x] Error: allocation failed in 'vector_emplace_back' at %s:%d\n", __FILE__, __LINE__); \
            } \
        } \
        if ((vec).size < (vec).capacity) \
            _slot = &(vec).data[(vec).size++]; \
    } \
    _slot; \
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
    TYPE_OF_VAL(*(vec).data) *_slot = private_vector_emplace_back_ptr(vec); \
    if (_slot) { \
        TYPE_OF_VAL(*(vec).data) _tmp = { __VA_ARGS__ }; \
        *_slot = _tmp; \
    } \
} while(0)

#define vector_size(vec)        ((vec).size)
#define vector_bytesize(vec)    ((vec).size * sizeof(*(vec).data))
#define vector_capacity(vec)    ((vec).capacity)
#define vector_empty(vec)       ((vec).size == 0)

/**
 * @brief Bounds-checked element access. Aborts on out-of-bounds or uninitialized vector.
 */
#define vector_at(vec, index) \
    (((vec).magic == VECTOR_MAGIC_INIT && (size_t)(index) < (vec).size) ? \
     (vec).data[(index)] : \
     (CLIB_PREFIX fprintf(stderr, "[x] Error: out-of-bounds access at %s:%d\n", __FILE__, __LINE__), \
      abort(), VECTOR_UNREACHABLE(), (vec).data[0]))

/**
 * @brief Returns the last element. Aborts on empty or uninitialized vector.
 */
#define vector_back(vec) \
    (((vec).magic == VECTOR_MAGIC_INIT && (vec).size > 0) ? \
     (vec).data[(vec).size - 1] : \
     (CLIB_PREFIX fprintf(stderr, "[x] Error: 'vector_back' on empty/uninitialized vector at %s:%d\n", __FILE__, __LINE__), \
      abort(), VECTOR_UNREACHABLE(), (vec).data[0]))

/**
 * @brief Returns the first element. Aborts on empty or uninitialized vector.
 */
#define vector_front(vec) \
    (((vec).magic == VECTOR_MAGIC_INIT && (vec).size > 0) ? \
     (vec).data[0] : \
     (CLIB_PREFIX fprintf(stderr, "[x] Error: 'vector_front' on empty/uninitialized vector at %s:%d\n", __FILE__, __LINE__), \
      abort(), VECTOR_UNREACHABLE(), (vec).data[0]))

/**
 * Removes the last element from the vector.
 */
#define vector_pop_back(vec) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before 'vector_pop_back' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((vec).size == 0)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: 'vector_pop_back' on empty vector at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    (vec).size--; \
} while(0)

/**
 * Resets vector size to 0 without freeing memory.
 */
#define vector_clear(vec) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before 'vector_clear' at %s:%d\n", __FILE__, __LINE__); \
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
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector already destroyed at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((vec).magic != VECTOR_MAGIC_INIT) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before destroy at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    free((vec).data); \
    (vec).data     = NULL_PTR; \
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
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before 'vector_reserve' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((new_capacity) <= (vec).capacity) break; \
    size_t _cap = (new_capacity) < 4 ? 4 : (size_t)(new_capacity); \
    TYPE_OF((vec).data) _nd = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _cap * sizeof(*(vec).data)); \
    if (LIKELY(_nd != NULL_PTR)) { \
        (vec).data     = _nd; \
        (vec).capacity = _cap; \
    } else { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: allocation failed in 'vector_reserve' at %s:%d\n", __FILE__, __LINE__); \
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
        CLIB_PREFIX fprintf(stderr, \
        "[x] Error: vector not initialized before 'vector_resize' at %s:%d\n", \
        __FILE__, __LINE__); \
        break; \
    } \
    if ((size_t)(new_size) > (vec).capacity) { \
        vector_reserve(vec, new_size); \
    if (UNLIKELY((vec).capacity < (size_t)(new_size))) break; \
    } \
    if ((size_t)(new_size) > (vec).size) { \
        TYPE_OF_VAL(*(vec).data) _fv = (def_val); \
    for (size_t _i = (vec).size; _i < (size_t)(new_size); _i++) \
        (vec).data[_i] = _fv; \
    } \
    (vec).size = (size_t)(new_size); \
} while(0)


/**
 * @brief Resizes the vector to `new_size` elements.
 *        ALL elements (existing + new) are overwritten with `def_val`.
 *        Useful for reinitializing the entire buffer to a known state.
 * @note  Release builds (-O2) auto-vectorize the fill loop to memset/SIMD.
 * @note  Unlike `vector_resize`, this always overwrites from index 0,
 *        so existing data is not preserved.
 */
#define vector_resize_full(vec, new_size, def_val) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, \
            "[x] Error: vector not initialized before 'vector_resize_full' at %s:%d\n", \
            __FILE__, __LINE__); \
        break; \
    } \
    if ((size_t)(new_size) > (vec).capacity) { \
        vector_reserve(vec, new_size); \
        if (UNLIKELY((vec).capacity < (size_t)(new_size))) break; \
    } \
    TYPE_OF_VAL(*(vec).data) _fv = (def_val); \
    for (size_t _i = 0; _i < (size_t)(new_size); _i++) \
        (vec).data[_i] = _fv; \
    (vec).size = (size_t)(new_size); \
} while(0)


/** 
 * Shrinks the internal buffer to match the current size exactly.
 * If the vector is empty, fully deallocates the buffer.
 */
#define vector_shrink_to_fit(vec) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before 'vector_shrink_to_fit' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if ((vec).size == (vec).capacity) break; \
    if ((vec).size == 0) { \
        free((vec).data); \
        (vec).data     = NULL_PTR; \
        (vec).capacity = 0; \
    } else { \
        TYPE_OF((vec).data) _nd = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, (vec).size * sizeof(*(vec).data)); \
        if (LIKELY(_nd != NULL_PTR)) { \
            (vec).data     = _nd; \
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
    ptrdiff_t _result = -1; \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_find_custom' at %s:%d\n", __FILE__, __LINE__); \
    } else { \
        TYPE_OF_VAL(*(vec).data) _search_val = (value); \
        size_t _sz = (vec).size; \
        size_t _i  = 0; \
        for (; _i + 3 < _sz; _i += 4) { \
            if (cmp_func((vec).data[_i],   _search_val)) { _result = (ptrdiff_t)_i;     break; } \
            if (cmp_func((vec).data[_i+1], _search_val)) { _result = (ptrdiff_t)(_i+1); break; } \
            if (cmp_func((vec).data[_i+2], _search_val)) { _result = (ptrdiff_t)(_i+2); break; } \
            if (cmp_func((vec).data[_i+3], _search_val)) { _result = (ptrdiff_t)(_i+3); break; } \
        } \
        if (_result == -1) { \
            for (; _i < _sz; ++_i) { \
                if (cmp_func((vec).data[_i], _search_val)) { \
                    _result = (ptrdiff_t)_i; \
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
    ptrdiff_t _result = -1; \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: Vector not initialized before 'vector_find' at %s:%d\n", __FILE__, __LINE__); \
    } else { \
        TYPE_OF_VAL(*(vec).data) _search_val = (value); \
        size_t _i  = 0; \
        size_t _sz = (vec).size; \
        for (; _i + 3 < _sz; _i += 4) { \
            if ((vec).data[_i]   == _search_val) { _result = (ptrdiff_t)_i;   break; } \
            if ((vec).data[_i+1] == _search_val) { _result = (ptrdiff_t)(_i+1); break; } \
            if ((vec).data[_i+2] == _search_val) { _result = (ptrdiff_t)(_i+2); break; } \
            if ((vec).data[_i+3] == _search_val) { _result = (ptrdiff_t)(_i+3); break; } \
        } \
        if (_result == -1) { \
            for (; _i < _sz; ++_i) { \
                if ((vec).data[_i] == _search_val) { \
                    _result = (ptrdiff_t)_i; \
                    break; \
                } \
            } \
        } \
    } \
    _result; \
})

/* !! PRIVATE !! — Do not call directly */
static inline int private_vector_push_back_args_inline
(
    void *vec_ptr, size_t elem_size,
    const void *elems, size_t count
) 

{
    VectorBase *vec = (VectorBase *)vec_ptr;
    if (UNLIKELY(vec->magic != VECTOR_MAGIC_INIT)) return -1;
    size_t new_size = vec->size + count;
    if (UNLIKELY(new_size > vec->capacity)) {
        size_t nc = vec->capacity == 0 ? (count < 4 ? 4 : count) : vec->capacity;
        while (nc < new_size) nc <<= 1;
        void *nd = CLIB_PREFIX realloc(vec->data, nc * elem_size);
        if (UNLIKELY(nd == NULL_PTR)) return -1;
        vec->data     = nd;
        vec->capacity = nc;
    }
    memcpy((char *)vec->data + vec->size * elem_size, elems, count * elem_size);
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
        CLIB_PREFIX fprintf(stderr, "[x] Error: 'vector_push_back_args' failed at %s:%d\n", __FILE__, __LINE__); \
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
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    size_t _pos = (size_t)(position); \
    if (UNLIKELY(_pos > (vec).size)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: insert position out of bounds in 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((vec).size >= (vec).capacity)) { \
        size_t _nc = VECTOR_GROW_CAPACITY((vec).capacity); \
        TYPE_OF((vec).data) _nd = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _nc * sizeof(*(vec).data)); \
        if (UNLIKELY(_nd == NULL_PTR)) { \
            CLIB_PREFIX fprintf(stderr, "[x] Error: allocation failed in 'vector_insert' at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
        (vec).data     = _nd; \
        (vec).capacity = _nc; \
    } \
    if (_pos < (vec).size) \
        memmove(&(vec).data[_pos + 1], &(vec).data[_pos], ((vec).size - _pos) * sizeof(*(vec).data)); \
    (vec).data[_pos] = (value); \
    (vec).size++; \
} while(0)

/**
 * Inserts a range of elements from an array at position pos.
 * @note O(N) due to memmove + memcpy.
 */
#define vector_insert_range(vec, pos, arr, count) do { \
    if (UNLIKELY((vec).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: vector not initialized before 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((arr) == NULL_PTR)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: source array is NULL in 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    size_t _pos = (size_t)(pos); \
    size_t _cnt = (size_t)(count); \
    if (UNLIKELY(_pos > (vec).size)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: insert position out of bounds in 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (_cnt == 0) break; \
    size_t _ns = (vec).size + _cnt; \
    if (_ns > (vec).capacity) { \
        size_t _nc = VECTOR_GROW_CAPACITY((vec).capacity); \
        if (_nc < _ns) _nc = _ns; \
        TYPE_OF((vec).data) _nd = (TYPE_OF((vec).data))CLIB_PREFIX realloc((vec).data, _nc * sizeof(*(vec).data)); \
        if (UNLIKELY(_nd == NULL_PTR)) { \
            CLIB_PREFIX fprintf(stderr, "[x] Error: allocation failed in 'vector_insert_range' at %s:%d\n", __FILE__, __LINE__); \
            break; \
        } \
        (vec).data     = _nd; \
        (vec).capacity = _nc; \
    } \
    if (_pos < (vec).size) \
        memmove(&(vec).data[_pos + _cnt], &(vec).data[_pos], ((vec).size - _pos) * sizeof(*(vec).data)); \
    memcpy(&(vec).data[_pos], (arr), _cnt * sizeof(*(vec).data)); \
    (vec).size = _ns; \
} while(0)

/* !! PRIVATE !! — Do not call directly */
static inline int private_vector_insert_args_inline
(
    void *vec_ptr, size_t elem_size,
    size_t index,
    const void *elems, size_t count
) 

{
    VectorBase *vec = (VectorBase *)vec_ptr;
    if (UNLIKELY(vec->magic != VECTOR_MAGIC_INIT)) return -1;
    if (UNLIKELY(index > vec->size))               return -1;
    size_t new_size = vec->size + count;
    if (UNLIKELY(new_size > vec->capacity)) {
        size_t nc = vec->capacity == 0 ? (count < 4 ? 4 : count) : vec->capacity;
        while (nc < new_size) nc <<= 1;
        void *nd = CLIB_PREFIX realloc(vec->data, nc * elem_size);
        if (UNLIKELY(nd == NULL_PTR)) return -1;
        vec->data     = nd;
        vec->capacity = nc;
    }
    memmove(
        (char *)vec->data + (index + count) * elem_size,
        (char *)vec->data + index * elem_size,
        (vec->size - index) * elem_size
    );
    memcpy((char *)vec->data + index * elem_size, elems, count * elem_size);
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
    VECTOR_ASSERT_SAME_TYPE(vec1, vec2); \
    if (UNLIKELY((vec1).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: first vector not initialized before 'vector_swap' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    if (UNLIKELY((vec2).magic != VECTOR_MAGIC_INIT)) { \
        CLIB_PREFIX fprintf(stderr, "[x] Error: second vector not initialized before 'vector_swap' at %s:%d\n", __FILE__, __LINE__); \
        break; \
    } \
    TYPE_OF((vec1).data) _td = (vec1).data;     (vec1).data     = (vec2).data;     (vec2).data     = _td; \
    size_t               _ts = (vec1).size;     (vec1).size     = (vec2).size;     (vec2).size     = _ts; \
    size_t               _tc = (vec1).capacity; (vec1).capacity = (vec2).capacity; (vec2).capacity = _tc; \
    \
} while(0)

#endif /* VECTOR_H */
