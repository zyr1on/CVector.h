# API REFERENCE


> **Tip:**  
> All macros are safe and print detailed errors to stderr if misused.  
> Always call `vector_init()` before use and `vector_destroy()` after use to avoid memory leaks.  
> Designed for performance and usability in C projects.  

## 🛠️ Function Reference

Below are the main functions/macros, their purpose, and usage. All macros print helpful error messages if misused!

## ⚠️ Notes

- Always call `vector_init` before use and `vector_destroy` when done to avoid leaks.
- Not thread-safe.
- All macros print descriptive errors on misuse to `stderr` for quick debugging.
- Optimized for performance and safety in C projects.
- No dependencies, just drop in and use!

---

Enjoy modern C vectors! 😊


```c
vector(type)                                 -> Declares a generic vector of the specified type (macro, no return).
vector_init(vec)                             -> Initializes the vector. Warns if already initialized. (void).
vector_is_valid(vec)                         -> Returns nonzero if the vector is properly initialized (macro, int).
vector_push_back(vec, value)                 -> Appends value to the end of the vector, grows if needed. (void, prints error on fail).
vector_push_back_args(vec, ...)              -> Appends multiple values at once. (void, prints error on fail).
vector_insert(vec, position, value)          -> Inserts elements at a specific position in the vector
vector_insert_range(vec, pos, arr, count)    -> Inserts a range of elements
vector_insert_args(vec, pos, first, ...)     -> Inserts one or more elements at the specified position in the vector.

int vector_at(vec, index)                    -> Returns the element at index. Bounds-checked (macro, element type).
int vector_size(vec)                         -> Returns number of elements in the vector (macro, size_t).
int vector_capacity(vec)                     -> Returns current allocated capacity (macro, size_t).
int vector_empty(vec)                        -> Returns 1 if vector is empty, 0 otherwise (macro, int).
int vector_find(vec, value)                  -> Returns index of first occurrence of value, or -1 if not found (macro, size_t).
int vector_find_custom(vec, value, cmp_func) -> Returns index of first occurrence using custom comparator, or -1 if not found (macro, int).

vector_back(vec)                             -> Returns the last element (macro, element type).
vector_front(vec)                            -> Returns the first element (macro, element type).
vector_pop_back(vec)                         -> Removes the last element. (void, prints error if not initialized/empty).
vector_clear(vec)                            -> Removes all elements but keeps memory allocated. (void, prints error if not initialized).
vector_destroy(vec)                          -> Frees all memory and marks vector as destroyed. (void, prints error if already destroyed or not initialized).
vector_reserve(vec, new_capacity)            -> Ensures capacity is at least new_capacity. (void, prints error on fail).
vector_resize(vec, new_size, def_val)        -> Changes vector size, fills new elements with default_value. (void, prints error on fail).
vector_shrink_to_fit(vec)                    -> Reduces capacity to match size, freeing unused memory. (void, prints error if not initialized).
vector_foreach(vec, item)                    -> Macro for iterating over elements; item is a pointer to each element.
vector_swap(vec1, vec2)                      -> Swaps vector contents
```

# COMPARE WITH C++ STL std::vector<T> v;

| Function                  | CVector.h                        | std::vector                         |
|---------------------------|----------------------------------|-------------------------------------|
| declare vector            | vector(T) vec; ✔️                  | std::vector<T> v; ✔️              |
| init                      | vector_init(vec) ✔️              | automatic ✔️                         |
| push_back                 | vector_push_back ✔️              | v.push_back ✔️                      |
| push_back (variadic)      | vector_push_back_args ✔️         | (C++20: v.insert w/ fold) ✔️/⚠️        |
| insert                    | vector_insert ✔️                 | v.insert ✔️                         |
| insert_range              | vector_insert_range ✔️           | v.insert(begin, arr, arr+count) ✔️ |
| insert_args               | vector_insert_args ✔️            | (C++20: variadic insert) ✔️/⚠️         |
| at                        | vector_at ✔️                     | v.at ✔️                             |
| operator[]                | vec.data[i] ✔️              | v[i] ✔️                             |
| size                      | vector_size ✔️                   | v.size() ✔️                         |
| capacity                  | vector_capacity ✔️               | v.capacity() ✔️                     |
| empty                     | vector_empty ✔️                  | v.empty() ✔️                        |
| find                      | vector_find ✔️                   | std::find (with std::begin) ✔️ (NOT std::vector feat)    |
| find_custom               | vector_find_custom ✔️            | std::find_if + lambda ✔️ (NOT std::vector feat)          |
| back                      | vector_back ✔️                   | v.back() ✔️                         |
| front                     | vector_front ✔️                  | v.front() ✔️                        |
| pop_back                  | vector_pop_back ✔️               | v.pop_back() ✔️                     |
| clear                     | vector_clear ✔️                  | v.clear() ✔️                        |
| destroy                   | vector_destroy ✔️                | automatic ✔️                 |
| reserve                   | vector_reserve ✔️               | v.reserve() ✔️                     |
| resize                    | vector_resize ✔️                | v.resize(new_size, def_val) ✔️     |
| shrink_to_fit             | vector_shrink_to_fit ✔️          | v.shrink_to_fit() ✔️               |
| foreach                   | vector_foreach ✔️                | range-based for ✔️                 |
| swap                     | vector_swap ✔️                  | v.swap() ✔️               |


> **Notes:**  
> ✔️ = Has a direct equivalent.  
> ❌ = Not directly available in STL.  
> ⚠️ = Possible with C++20 or workaround.  
