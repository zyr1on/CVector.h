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
    - Safety: Bounds-checked access (vector_at/front/back), initialization tracking,
              overflow-checked growth. Programmer-error checks are removed in
              release builds (-DNDEBUG); see "Build Modes" below.
    - Modern: Includes handy iteration and batch operations.

    @par API overview
    - Lifetime:  vector_init, vector_destroy, vector_copy (deep), vector_swap
    - Add:       vector_push_back, vector_emplace_back, vector_push_back_args,
                 vector_insert, vector_insert_args, vector_insert_range
    - Remove:    vector_pop_back, vector_erase, vector_erase_range, vector_clear
    - Access:    vector_at, vector_front, vector_back, vec.data[i], vector_foreach
    - Search:    vector_find, vector_find_custom
    - Capacity:  vector_size, vector_capacity, vector_empty, vector_bytesize,
                 vector_reserve, vector_resize, vector_resize_full, vector_shrink_to_fit
    - Config:    VECTOR_NO_CHECKS / VECTOR_CHECKS / NDEBUG, VECTOR_ON_ERROR()

    @warning
    - Always initialize your vector with `vector_init()` before use.
    - Always destroy your vector with `vector_destroy()` after use to avoid memory leaks.
    - `b = a;` copies only the struct (shared buffer -> double free). Use vector_copy.
    - Requires GCC or Clang: uses GNU statement expressions `({ ... })` and `__typeof__`.
      Works with -std=c99/c11/c17/c23 (also with -pedantic) and gnu* modes in C,
      and C++11 or later in C++ (including inside templates).
      MSVC is not supported.
    - All macros may evaluate the `vec` argument more than once. Pass a plain
      variable (or `*ptr`), not an expression with side effects.
      Other arguments (value, index, size, ...) are evaluated exactly once.
*/

#ifndef VECTOR_H
#define VECTOR_H

#if !defined(__GNUC__) && !defined(__clang__)
    #error "vector.h requires GCC or Clang (GNU statement expressions and __typeof__)"
#endif

/**
 * @section Platform Abstraction Layer
 * -------------------------------------------------------------------------
 * This section provides a bridge between C and Modern C++ compilers.
 * It handles differences in type deduction, namespace management, and
 * strict type-checking rules.
 */

#ifdef __cplusplus
/* C++ Compiler Configuration */
    #include <cstdio>
    #include <cstdlib>
    #include <cstddef>
    #include <cstdint>
    #include <cstring>
    #include <type_traits>
    /**
    * @brief C++ Namespace and Type Deduction Macros
    * C++ `decltype(*ptr)` returns a reference (T&), which cannot be used for
    * array declarations. VECTOR_TYPE_OF_VAL cleans the reference to get the raw type.
    */
    #define VECTOR_STD               std::
    #define VECTOR_TYPE_OF(x)        decltype(x)
    /* `typename` makes the macros usable inside templates (dependent types). */
    #define VECTOR_TYPE_OF_VAL(x)    typename std::remove_reference<decltype(x)>::type
    #define VECTOR_NULL              nullptr
#else
/* Standard C Compiler Configuration */
    #include <stdio.h>
    #include <stdlib.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <string.h>
    /**
    * @brief typeof Support
    * `__typeof__` is available in every GCC/Clang C mode (unlike `typeof`,
    * which needs -std=gnu* or C23).
    */
    #define VECTOR_STD
    #define VECTOR_TYPE_OF(x)        __typeof__(x)
    #define VECTOR_TYPE_OF_VAL(x)    __typeof__(x)
    #define VECTOR_NULL              NULL
#endif

#define VECTOR_LIKELY(x)    __builtin_expect(!!(x), 1)
#define VECTOR_UNLIKELY(x)  __builtin_expect(!!(x), 0)

/**
 * @section Build Modes (checks on / off)
 * -------------------------------------------------------------------------
 * Checks for programmer errors (uninitialized vector, out-of-bounds index,
 * empty pop/front/back, bad insert position, overlapping insert source,
 * double destroy) follow the same rule as assert():
 *
 *   - Debug   (no NDEBUG)              -> checks ON
 *   - Release (-DNDEBUG, e.g. CMake Release) -> checks OFF
 *
 * Override with:
 *   -DVECTOR_NO_CHECKS   force checks OFF in any build
 *   -DVECTOR_CHECKS      force checks ON even with NDEBUG
 *
 * Allocation-failure and size-overflow checks are ALWAYS kept: those are
 * runtime conditions, not programmer errors, and they sit on the cold path.
 *
 * With checks OFF, misuse is undefined behaviour (like std::vector::operator[]).
 */
#if defined(VECTOR_NO_CHECKS)
    #define VECTOR_CHECKS_ENABLED 0
#elif defined(VECTOR_CHECKS) || !defined(NDEBUG)
    #define VECTOR_CHECKS_ENABLED 1
#else
    #define VECTOR_CHECKS_ENABLED 0
#endif

/* Internal: true when a programmer-error check is enabled and fails.
 * With checks OFF this is a constant 0 and the whole branch is removed. */
#define private_vector_check(cond) (VECTOR_CHECKS_ENABLED && VECTOR_UNLIKELY(cond))

/**
 * @brief Error hook, called after every error message (programmer errors
 *        while checks are ON, and allocation / overflow failures always).
 *        Default: do nothing (the operation is skipped and execution continues).
 *
 * @example Stop at the first error (handy in tests / debugging):
 *   #define VECTOR_ON_ERROR() abort()
 *   #include "vector.h"
 */
#ifndef VECTOR_ON_ERROR
    #define VECTOR_ON_ERROR() ((void)0)
#endif

/* Internal: prints "[x] Error: <msg> at file:line" to stderr, then runs the hook. */
#define private_vector_error(msg) \
    (VECTOR_STD fprintf(stderr, "[x] Error: " msg " at %s:%d\n", __FILE__, __LINE__), \
     VECTOR_ON_ERROR())

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
            std::is_trivially_copyable<typename std::remove_pointer<VECTOR_TYPE_OF((vec).data)>::type>::value, \
            "vector<T>: T must be trivially copyable — realloc is unsafe for types with constructors/destructors" \
        )
#else
    #define VECTOR_ASSERT_TRIVIAL(vec) /* no-op in C */
#endif


/**
 * @brief Enforces identical element types between two vectors at compile time.
 * @note  In C, element sizes are compared here; assigning the data pointers in
 *        vector_swap additionally triggers an incompatible-pointer diagnostic
 *        for different element types.
 */
#ifdef __cplusplus
    #define VECTOR_ASSERT_SAME_TYPE(v1, v2) \
        static_assert( \
            std::is_same< \
                typename std::remove_pointer<VECTOR_TYPE_OF((v1).data)>::type, \
                typename std::remove_pointer<VECTOR_TYPE_OF((v2).data)>::type \
            >::value, \
            "vector_swap: both vectors must store the same type" \
        )
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define VECTOR_ASSERT_SAME_TYPE(v1, v2) \
        _Static_assert( \
            sizeof(*(v1).data) == sizeof(*(v2).data), \
            "both vectors must store the same type (element size mismatch)" \
        )
#else
    /* C99: no _Static_assert; a negative array size fails compilation instead. */
    #define VECTOR_ASSERT_SAME_TYPE(v1, v2) \
        ((void)sizeof(char[(sizeof(*(v1).data) == sizeof(*(v2).data)) ? 1 : -1]))
#endif


/**
* @section Vector Core Metadata
* -------------------------------------------------------------------------
* Magic numbers used for initialization tracking and memory safety.
*/

#define VECTOR_MAGIC_INIT      0xDEADBEEF
#define VECTOR_MAGIC_DESTROYED 0xFEEDFACE

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

/* !! PRIVATE !! — Do not call directly.
 * Grows a buffer so that it holds at least `needed` elements.
 * Strategy: double the capacity (minimum 4) until it fits; if doubling would
 * overflow, fall back to exactly `needed`.
 * Returns the new buffer and writes the new capacity, or returns NULL
 * (old buffer untouched) on overflow / allocation failure.
 * Marked cold + noinline: growth is rare (amortized), so keeping it out of
 * line keeps every push_back / insert call site small. */
__attribute__((cold, noinline, unused))
static void *private_vector_grow
(
    void *data, size_t elem_size,
    size_t capacity, size_t needed,
    size_t *out_capacity
)
{
    size_t nc = capacity < 4 ? 4 : capacity;
    while (nc < needed) {
        if (nc > SIZE_MAX / 2) { nc = needed; break; }
        nc <<= 1;
    }
    if (nc > SIZE_MAX / elem_size) {
        nc = needed;
        if (nc > SIZE_MAX / elem_size) return VECTOR_NULL;
    }
    void *nd = VECTOR_STD realloc(data, nc * elem_size);
    if (nd != VECTOR_NULL) *out_capacity = nc;
    return nd;
}

/**
 * Initializes the vector structure.
 * @param vec The vector structure to initialize.
 *
 * @note The fields are NOT read before being written. A freshly declared
 *       `vector(T) v;` holds garbage, and reading it (e.g. checking `magic`)
 *       is undefined behaviour; if the garbage happened to equal
 *       VECTOR_MAGIC_INIT, initialization would be skipped and `v` would keep
 *       a stale data pointer. Calling vector_init on an already active vector
 *       therefore leaks its buffer — call vector_destroy first.
 */
#define vector_init(vec) do { \
    VECTOR_ASSERT_TRIVIAL(vec); \
    (vec).data     = VECTOR_NULL; \
    (vec).size     = 0; \
    (vec).capacity = 0; \
    (vec).magic    = VECTOR_MAGIC_INIT; \
} while(0)

/**
 * Appends an element to the end of the vector, automatically resizing the
 * internal buffer if the capacity is exceeded.
 * @param vec   The vector structure to modify.
 * @param value The element to be added (must match the vector's type).
 *              It may refer to the vector itself (e.g. vector_back(vec)).
 */
#define vector_push_back(vec, value) do { \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_push_back'"); \
        break; \
    } \
    /* Evaluate value first: it may read the vector itself, \
       which must happen before realloc and before size changes. */ \
    VECTOR_TYPE_OF_VAL(*(vec).data) _vec_pb_val = (value); \
    if (VECTOR_UNLIKELY((vec).size >= (vec).capacity)) { \
        size_t _vec_pb_nc = 0; \
        void *_vec_pb_nd = private_vector_grow((vec).data, sizeof(*(vec).data), \
                                               (vec).capacity, (vec).size + 1, &_vec_pb_nc); \
        if (VECTOR_UNLIKELY(_vec_pb_nd == VECTOR_NULL)) { \
            private_vector_error("allocation failed in 'vector_push_back'"); \
            break; \
        } \
        (vec).data     = (VECTOR_TYPE_OF((vec).data))_vec_pb_nd; \
        (vec).capacity = _vec_pb_nc; \
    } \
    (vec).data[(vec).size] = _vec_pb_val; \
    (vec).size++; \
} while(0)

/**
 * Returns a pointer to the next available slot in the vector, resizing memory
 * if necessary. Allows for zero-copy construction of elements directly
 * into the vector's buffer.
 * @param vec The vector structure to modify.
 * @return    A pointer to the newly reserved slot, or NULL on failure.
 */
#define private_vector_emplace_back_ptr(vec) __extension__ ({ \
    VECTOR_TYPE_OF_VAL((vec).data) _vec_ep_slot = VECTOR_NULL; \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_emplace_back'"); \
    } else { \
        int _vec_ep_ok = 1; \
        if (VECTOR_UNLIKELY((vec).size >= (vec).capacity)) { \
            size_t _vec_ep_nc = 0; \
            void *_vec_ep_nd = private_vector_grow((vec).data, sizeof(*(vec).data), \
                                                   (vec).capacity, (vec).size + 1, &_vec_ep_nc); \
            if (VECTOR_LIKELY(_vec_ep_nd != VECTOR_NULL)) { \
                (vec).data     = (VECTOR_TYPE_OF((vec).data))_vec_ep_nd; \
                (vec).capacity = _vec_ep_nc; \
            } else { \
                private_vector_error("allocation failed in 'vector_emplace_back'"); \
                _vec_ep_ok = 0; \
            } \
        } \
        if (_vec_ep_ok) \
            _vec_ep_slot = &(vec).data[(vec).size++]; \
    } \
    _vec_ep_slot; \
})

/**
 * Constructs an object in-place at the end of the vector.
 *
 * @note C: Supports designated initializers in any order:
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
 * Uses `Type _tmp = {...}` instead of a C compound literal (Type){...},
 * which works in both C and C++.
 */
#define vector_emplace_back(vec, ...) do { \
    /* Build the element before reserving the slot, so arguments that read \
       the vector see its state before realloc / size++. */ \
    VECTOR_TYPE_OF_VAL(*(vec).data) _vec_eb_tmp = { __VA_ARGS__ }; \
    VECTOR_TYPE_OF_VAL(*(vec).data) *_vec_eb_slot = private_vector_emplace_back_ptr(vec); \
    if (_vec_eb_slot) \
        *_vec_eb_slot = _vec_eb_tmp; \
} while(0)

#define vector_size(vec)        ((vec).size)
#define vector_bytesize(vec)    ((vec).size * sizeof(*(vec).data))
#define vector_capacity(vec)    ((vec).capacity)
#define vector_empty(vec)       ((vec).size == 0)

/**
 * @brief Bounds-checked element access. Aborts on out-of-bounds or uninitialized vector.
 * @note  Returns an lvalue: `vector_at(v, i) = x;` and `&vector_at(v, i)` both work.
 */
#define vector_at(vec, index) (*__extension__ ({ \
    size_t _vec_at_i = (size_t)(index); \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_at'"); \
        VECTOR_STD abort(); \
    } \
    if (private_vector_check(_vec_at_i >= (vec).size)) { \
        VECTOR_STD fprintf(stderr, "[x] Error: 'vector_at' index %llu out of bounds (size %llu) at %s:%d\n", \
                           (unsigned long long)_vec_at_i, (unsigned long long)(vec).size, __FILE__, __LINE__); \
        VECTOR_STD abort(); \
    } \
    &(vec).data[_vec_at_i]; \
}))

/**
 * @brief Returns the last element (lvalue). Aborts on empty or uninitialized vector.
 */
#define vector_back(vec) (*__extension__ ({ \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT || (vec).size == 0)) { \
        private_vector_error("'vector_back' on empty/uninitialized vector"); \
        VECTOR_STD abort(); \
    } \
    &(vec).data[(vec).size - 1]; \
}))

/**
 * @brief Returns the first element (lvalue). Aborts on empty or uninitialized vector.
 */
#define vector_front(vec) (*__extension__ ({ \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT || (vec).size == 0)) { \
        private_vector_error("'vector_front' on empty/uninitialized vector"); \
        VECTOR_STD abort(); \
    } \
    &(vec).data[0]; \
}))

/**
 * Removes the last element from the vector.
 */
#define vector_pop_back(vec) do { \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_pop_back'"); \
        break; \
    } \
    if (private_vector_check((vec).size == 0)) { \
        private_vector_error("'vector_pop_back' on empty vector"); \
        break; \
    } \
    (vec).size--; \
} while(0)

/**
 * Resets vector size to 0 without freeing memory.
 */
#define vector_clear(vec) do { \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_clear'"); \
        break; \
    } \
    (vec).size = 0; \
} while(0)

/**
 * Deallocates the vector's underlying memory buffer and resets its state.
 * Implements safeguards against double-free and uninitialized destroy.
 * A destroyed vector may be initialized again with vector_init.
 * @param vec The vector structure to destroy.
 */
#define vector_destroy(vec) do { \
    if (private_vector_check((vec).magic == VECTOR_MAGIC_DESTROYED)) { \
        private_vector_error("vector already destroyed"); \
        break; \
    } \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before destroy"); \
        break; \
    } \
    VECTOR_STD free((vec).data); \
    (vec).data     = VECTOR_NULL; \
    (vec).size     = 0; \
    (vec).capacity = 0; \
    (vec).magic    = VECTOR_MAGIC_DESTROYED; \
} while(0)

/**
 * Reserves capacity for at least new_capacity elements (minimum 4).
 * No-op if current capacity already satisfies the request.
 */
#define vector_reserve(vec, new_capacity) do { \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_reserve'"); \
        break; \
    } \
    size_t _vec_rv_cap = (size_t)(new_capacity); \
    if (_vec_rv_cap <= (vec).capacity) break; \
    if (_vec_rv_cap < 4) _vec_rv_cap = 4; \
    if (VECTOR_UNLIKELY(_vec_rv_cap > SIZE_MAX / sizeof(*(vec).data))) { \
        private_vector_error("requested capacity too large in 'vector_reserve'"); \
        break; \
    } \
    void *_vec_rv_nd = VECTOR_STD realloc((vec).data, _vec_rv_cap * sizeof(*(vec).data)); \
    if (VECTOR_LIKELY(_vec_rv_nd != VECTOR_NULL)) { \
        (vec).data     = (VECTOR_TYPE_OF((vec).data))_vec_rv_nd; \
        (vec).capacity = _vec_rv_cap; \
    } else { \
        private_vector_error("allocation failed in 'vector_reserve'"); \
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
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_resize'"); \
        break; \
    } \
    size_t _vec_rs_n = (size_t)(new_size); \
    VECTOR_TYPE_OF_VAL(*(vec).data) _vec_rs_fv = (def_val); \
    if (_vec_rs_n > (vec).capacity) { \
        vector_reserve(vec, _vec_rs_n); \
        if (VECTOR_UNLIKELY((vec).capacity < _vec_rs_n)) break; \
    } \
    for (size_t _vec_rs_i = (vec).size; _vec_rs_i < _vec_rs_n; _vec_rs_i++) \
        (vec).data[_vec_rs_i] = _vec_rs_fv; \
    (vec).size = _vec_rs_n; \
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
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_resize_full'"); \
        break; \
    } \
    size_t _vec_rf_n = (size_t)(new_size); \
    VECTOR_TYPE_OF_VAL(*(vec).data) _vec_rf_fv = (def_val); \
    if (_vec_rf_n > (vec).capacity) { \
        vector_reserve(vec, _vec_rf_n); \
        if (VECTOR_UNLIKELY((vec).capacity < _vec_rf_n)) break; \
    } \
    for (size_t _vec_rf_i = 0; _vec_rf_i < _vec_rf_n; _vec_rf_i++) \
        (vec).data[_vec_rf_i] = _vec_rf_fv; \
    (vec).size = _vec_rf_n; \
} while(0)


/**
 * Shrinks the internal buffer to match the current size exactly.
 * If the vector is empty, fully deallocates the buffer.
 */
#define vector_shrink_to_fit(vec) do { \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_shrink_to_fit'"); \
        break; \
    } \
    if ((vec).size == (vec).capacity) break; \
    if ((vec).size == 0) { \
        VECTOR_STD free((vec).data); \
        (vec).data     = VECTOR_NULL; \
        (vec).capacity = 0; \
    } else { \
        void *_vec_sf_nd = VECTOR_STD realloc((vec).data, (vec).size * sizeof(*(vec).data)); \
        if (VECTOR_LIKELY(_vec_sf_nd != VECTOR_NULL)) { \
            (vec).data     = (VECTOR_TYPE_OF((vec).data))_vec_sf_nd; \
            (vec).capacity = (vec).size; \
        } \
    } \
} while(0)

/**
 * Iterates over each element in the vector using a pointer.
 * @warning Do NOT modify vector size (push/resize/pop) inside this loop.
 * @param vec  The vector to iterate over.
 * @param item Iterator pointer variable name.
 */
#define vector_foreach(vec, item) \
    for (VECTOR_TYPE_OF_VAL(*(vec).data) *item = (vec).data, \
             *item##_vec_end = (item != VECTOR_NULL) ? item + (vec).size : item; \
         item != item##_vec_end; \
         ++item)

/**
 * Searches using a custom comparison predicate.
 * Optimized with 4-step loop unrolling.
 * @param vec      The vector to search.
 * @param value    The target value to find.
 * @param cmp_func Comparison function: cmp_func(element, value) -> nonzero if match.
 *                 Both arguments are passed by value.
 * @return Index of first match (ptrdiff_t), or -1 if not found.
 */
#define vector_find_custom(vec, value, cmp_func) __extension__ ({ \
    ptrdiff_t _vec_fc_res = -1; \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_find_custom'"); \
    } else { \
        VECTOR_TYPE_OF_VAL(*(vec).data) _vec_fc_val = (value); \
        size_t _vec_fc_sz = (vec).size; \
        size_t _vec_fc_i  = 0; \
        for (; _vec_fc_i + 3 < _vec_fc_sz; _vec_fc_i += 4) { \
            if (cmp_func((vec).data[_vec_fc_i],   _vec_fc_val)) { _vec_fc_res = (ptrdiff_t)_vec_fc_i;     break; } \
            if (cmp_func((vec).data[_vec_fc_i+1], _vec_fc_val)) { _vec_fc_res = (ptrdiff_t)(_vec_fc_i+1); break; } \
            if (cmp_func((vec).data[_vec_fc_i+2], _vec_fc_val)) { _vec_fc_res = (ptrdiff_t)(_vec_fc_i+2); break; } \
            if (cmp_func((vec).data[_vec_fc_i+3], _vec_fc_val)) { _vec_fc_res = (ptrdiff_t)(_vec_fc_i+3); break; } \
        } \
        if (_vec_fc_res == -1) { \
            for (; _vec_fc_i < _vec_fc_sz; ++_vec_fc_i) { \
                if (cmp_func((vec).data[_vec_fc_i], _vec_fc_val)) { \
                    _vec_fc_res = (ptrdiff_t)_vec_fc_i; \
                    break; \
                } \
            } \
        } \
    } \
    _vec_fc_res; \
})

/**
 * Searches using == operator. Optimized with 4-step loop unrolling.
 * @note Only valid for scalar types (int, float, pointer, enum).
 *       Do NOT use with structs or strings — use vector_find_custom instead.
 * @return Index of first match (ptrdiff_t), or -1 if not found.
 */
#define vector_find(vec, value) __extension__ ({ \
    ptrdiff_t _vec_fd_res = -1; \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before 'vector_find'"); \
    } else { \
        VECTOR_TYPE_OF_VAL(*(vec).data) _vec_fd_val = (value); \
        size_t _vec_fd_i  = 0; \
        size_t _vec_fd_sz = (vec).size; \
        for (; _vec_fd_i + 3 < _vec_fd_sz; _vec_fd_i += 4) { \
            if ((vec).data[_vec_fd_i]   == _vec_fd_val) { _vec_fd_res = (ptrdiff_t)_vec_fd_i;     break; } \
            if ((vec).data[_vec_fd_i+1] == _vec_fd_val) { _vec_fd_res = (ptrdiff_t)(_vec_fd_i+1); break; } \
            if ((vec).data[_vec_fd_i+2] == _vec_fd_val) { _vec_fd_res = (ptrdiff_t)(_vec_fd_i+2); break; } \
            if ((vec).data[_vec_fd_i+3] == _vec_fd_val) { _vec_fd_res = (ptrdiff_t)(_vec_fd_i+3); break; } \
        } \
        if (_vec_fd_res == -1) { \
            for (; _vec_fd_i < _vec_fd_sz; ++_vec_fd_i) { \
                if ((vec).data[_vec_fd_i] == _vec_fd_val) { \
                    _vec_fd_res = (ptrdiff_t)_vec_fd_i; \
                    break; \
                } \
            } \
        } \
    } \
    _vec_fd_res; \
})

/* !! PRIVATE !! — Do not call directly.
 * Inserts `count` elements from `src` at position `pos`.
 * `fname` is a string literal used in error messages.
 * `src` must have the vector's element type and must not point into the vector. */
#define private_vector_insert_n(vec, pos, src, count, fname) do { \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before '" fname "'"); \
        break; \
    } \
    const VECTOR_TYPE_OF_VAL(*(vec).data) *_vec_in_src = (src); \
    size_t _vec_in_pos = (size_t)(pos); \
    size_t _vec_in_cnt = (size_t)(count); \
    if (private_vector_check(_vec_in_pos > (vec).size)) { \
        private_vector_error("insert position out of bounds in '" fname "'"); \
        break; \
    } \
    if (_vec_in_cnt == 0) break; \
    if (private_vector_check(_vec_in_src == VECTOR_NULL)) { \
        private_vector_error("source array is NULL in '" fname "'"); \
        break; \
    } \
    if (private_vector_check((vec).data != VECTOR_NULL && \
            (uintptr_t)_vec_in_src >= (uintptr_t)(vec).data && \
            (uintptr_t)_vec_in_src <  (uintptr_t)((vec).data + (vec).size))) { \
        private_vector_error("source array points into the vector itself in '" fname "'"); \
        break; \
    } \
    if (VECTOR_UNLIKELY(_vec_in_cnt > SIZE_MAX - (vec).size)) { \
        private_vector_error("size overflow in '" fname "'"); \
        break; \
    } \
    size_t _vec_in_ns = (vec).size + _vec_in_cnt; \
    if (_vec_in_ns > (vec).capacity) { \
        size_t _vec_in_nc = 0; \
        void *_vec_in_nd = private_vector_grow((vec).data, sizeof(*(vec).data), \
                                               (vec).capacity, _vec_in_ns, &_vec_in_nc); \
        if (VECTOR_UNLIKELY(_vec_in_nd == VECTOR_NULL)) { \
            private_vector_error("allocation failed in '" fname "'"); \
            break; \
        } \
        (vec).data     = (VECTOR_TYPE_OF((vec).data))_vec_in_nd; \
        (vec).capacity = _vec_in_nc; \
    } \
    if (_vec_in_pos < (vec).size) \
        VECTOR_STD memmove(&(vec).data[_vec_in_pos + _vec_in_cnt], &(vec).data[_vec_in_pos], \
                           ((vec).size - _vec_in_pos) * sizeof(*(vec).data)); \
    VECTOR_STD memcpy(&(vec).data[_vec_in_pos], _vec_in_src, _vec_in_cnt * sizeof(*(vec).data)); \
    (vec).size = _vec_in_ns; \
} while(0)

/**
 * Appends multiple elements to the vector in a single operation.
 * @note Efficient for batch updates: single resize check, bulk memcpy.
 * @param vec The vector structure to modify.
 * @param ... Comma-separated values matching the vector's element type.
 */
#define vector_push_back_args(vec, ...) do { \
    VECTOR_TYPE_OF_VAL(*(vec).data) _vec_pa_tmp[] = { __VA_ARGS__ }; \
    private_vector_insert_n(vec, (vec).size, _vec_pa_tmp, \
                            sizeof(_vec_pa_tmp) / sizeof(_vec_pa_tmp[0]), "vector_push_back_args"); \
} while(0)

/**
 * Inserts value at position, shifting subsequent elements right.
 * @note O(N) complexity due to memmove. Use sparingly on large vectors.
 * @param vec      The vector to modify.
 * @param position Insert index (0 to size).
 * @param value    Element to insert. It may refer to an element of this vector.
 */
#define vector_insert(vec, position, value) do { \
    /* Copy value first: it may refer to an element that will move. */ \
    VECTOR_TYPE_OF_VAL(*(vec).data) _vec_ins_val = (value); \
    private_vector_insert_n(vec, position, &_vec_ins_val, 1, "vector_insert"); \
} while(0)

/**
 * Inserts a range of elements from an array at position pos.
 * @note O(N) due to memmove + memcpy.
 * @param arr Pointer to elements of the vector's type. Must NOT point into
 *            the vector itself (rejected with an error).
 */
#define vector_insert_range(vec, pos, arr, count) \
    private_vector_insert_n(vec, pos, arr, count, "vector_insert_range")

/**
 * Inserts multiple values at index using variadic arguments.
 */
#define vector_insert_args(vec, idx, ...) do { \
    VECTOR_TYPE_OF_VAL(*(vec).data) _vec_ia_tmp[] = { __VA_ARGS__ }; \
    private_vector_insert_n(vec, idx, _vec_ia_tmp, \
                            sizeof(_vec_ia_tmp) / sizeof(_vec_ia_tmp[0]), "vector_insert_args"); \
} while(0)

/**
 * Removes `count` elements starting at position `pos`, shifting the rest left.
 * Capacity is unchanged.
 * @note O(N) due to memmove.
 * @param vec   The vector to modify.
 * @param pos   Index of the first element to remove.
 * @param count Number of elements to remove (pos + count must be <= size).
 */
#define vector_erase_range(vec, pos, count) \
    private_vector_erase_n(vec, pos, count, "vector_erase_range")

/**
 * Removes the element at position `pos`, shifting the rest left.
 * @note O(N) due to memmove. To remove the last element use vector_pop_back.
 */
#define vector_erase(vec, pos) \
    private_vector_erase_n(vec, pos, 1, "vector_erase")

/* !! PRIVATE !! — Do not call directly. */
#define private_vector_erase_n(vec, pos, count, fname) do { \
    if (private_vector_check((vec).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("vector not initialized before '" fname "'"); \
        break; \
    } \
    size_t _vec_er_pos = (size_t)(pos); \
    size_t _vec_er_cnt = (size_t)(count); \
    if (private_vector_check(_vec_er_pos > (vec).size || _vec_er_cnt > (vec).size - _vec_er_pos)) { \
        private_vector_error("range out of bounds in '" fname "'"); \
        break; \
    } \
    if (_vec_er_cnt == 0) break; \
    VECTOR_STD memmove(&(vec).data[_vec_er_pos], &(vec).data[_vec_er_pos + _vec_er_cnt], \
                       ((vec).size - _vec_er_pos - _vec_er_cnt) * sizeof(*(vec).data)); \
    (vec).size -= _vec_er_cnt; \
} while(0)

/**
 * Deep copy: replaces the contents of `dst` with a copy of `src`'s elements.
 * Both vectors must be initialized and store the same type. `dst` keeps its
 * own buffer (grown if needed); `src` is unchanged.
 *
 * @note Plain assignment `dst = src;` is a SHALLOW copy: both would share one
 *       buffer, leading to double free on destroy. Use vector_copy instead.
 */
#define vector_copy(dst, src) do { \
    VECTOR_ASSERT_SAME_TYPE(dst, src); \
    if (private_vector_check((dst).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("destination vector not initialized before 'vector_copy'"); \
        break; \
    } \
    if (private_vector_check((src).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("source vector not initialized before 'vector_copy'"); \
        break; \
    } \
    /* Type check in C: assigning pointers of different types is diagnosed. */ \
    VECTOR_TYPE_OF((dst).data) _vec_cp_src = (src).data; \
    size_t _vec_cp_n = (src).size; \
    if ((void *)&(dst) == (void *)&(src)) break; \
    if (_vec_cp_n > (dst).capacity) { \
        size_t _vec_cp_nc = 0; \
        void *_vec_cp_nd = private_vector_grow((dst).data, sizeof(*(dst).data), \
                                               (dst).capacity, _vec_cp_n, &_vec_cp_nc); \
        if (VECTOR_UNLIKELY(_vec_cp_nd == VECTOR_NULL)) { \
            private_vector_error("allocation failed in 'vector_copy'"); \
            break; \
        } \
        (dst).data     = (VECTOR_TYPE_OF((dst).data))_vec_cp_nd; \
        (dst).capacity = _vec_cp_nc; \
    } \
    if (_vec_cp_n > 0) \
        VECTOR_STD memcpy((dst).data, _vec_cp_src, _vec_cp_n * sizeof(*(dst).data)); \
    (dst).size = _vec_cp_n; \
} while(0)

/**
 * Swaps contents of two vectors in O(1) — pointer and metadata swap, no deep copy.
 * @note Both vectors must store elements of the same type (checked at compile time).
 */
#define vector_swap(vec1, vec2) do { \
    VECTOR_ASSERT_SAME_TYPE(vec1, vec2); \
    if (private_vector_check((vec1).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("first vector not initialized before 'vector_swap'"); \
        break; \
    } \
    if (private_vector_check((vec2).magic != VECTOR_MAGIC_INIT)) { \
        private_vector_error("second vector not initialized before 'vector_swap'"); \
        break; \
    } \
    VECTOR_TYPE_OF((vec1).data) _vec_sw_d = (vec1).data;     (vec1).data     = (vec2).data;     (vec2).data     = _vec_sw_d; \
    size_t                      _vec_sw_s = (vec1).size;     (vec1).size     = (vec2).size;     (vec2).size     = _vec_sw_s; \
    size_t                      _vec_sw_c = (vec1).capacity; (vec1).capacity = (vec2).capacity; (vec2).capacity = _vec_sw_c; \
} while(0)

#endif /* VECTOR_H */
