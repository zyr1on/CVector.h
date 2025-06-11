# CVector.h – Simple & Safe Generic Vector Library for C 🚀

This header-only library provides a fast, safe, and easy-to-use generic dynamic array (vector) implementation for C. With modern features and robust error checks, you can enjoy the flexibility of C++-style vectors right in C, with zero dependencies and zero runtime overhead for most operations.

---

## ✨ Features

- 🛡️ **Type-safe Macros:** Create vectors of any type with compile-time safety.
- 🔥 **Branch Prediction and Fast Growth:** Uses bitwise tricks for efficient resizing.
- 🧩 **Debug Checks:** Catches double init/free, out-of-bounds, and misuse.
- ♻️ **No Hidden Allocations:** All memory management is explicit and safe.
---

## 🚀 Quick Start

```c
#include "CVector.h"

vector(int) v;
vector_init(v);
vector_push_back(v, 42);
printf("%d\n", vector_at(v, 0)); // 42
vector_destroy(v);
```

---
---


## Example

<details>
<summary>EXAMPLE with 3d vector</summary>
  


```c
#include "CVector.h"

typedef struct {
    float x;
    float y;
    float z;
}vec3;

int main(void)
{
    vector(vec3) container;
    vector_init(container); // we should initialize vector first.

    vec3 a = {3.2,2.1, 33.59};
    vec3 b = {0.0,0.0,0.0};

    vector_push_back(container,a);
    vector_push_back(container,b);
    vector_push_back_args(container,{1,2,3},{1,2,4},{1,2,32});

    vector_foreach(container,p){
        printf("(%.2f, %.2f, %.2f)\n", p->x,p->y, p->z);
    }

/*!
@note: or we can do it the traditional way

    for(int i=0; i < vector_size(container); i++) {
        printf("(%.2f, %.2f, %.2f)\n", container.data[i].x,container.data[i].y,container.data[i].z);
    }
*/

    vector_destroy(container); // !!IMPORTANT!! dont forget to destroy vector to avoid memory leaks
    
    return 0;
}
```

```bash
OUTPUT:
(3.20, 2.10, 33.59)
(0.00, 0.00, 0.00)
(1.00, 2.00, 3.00)
(1.00, 2.00, 4.00)
(1.00, 2.00, 32.00)
```
</details>

<details>
<summary>EXAMPLE with char* (string) </summary>
  
```c
#include "CVector.h"

typedef char* string;

int main(void)
{
    vector(string) vec;
    vector_init(vec);

    vector_push_back(vec,"hello");
    vector_push_back(vec,"world");
    vector_push_back_args(vec,"just","simple","vector");

    vector_foreach(vec, item)
        printf("%s\n", *item);

    vector_destroy(vec);

    return 0;
}
```
```BASH
OUTPUT:
hello
world
just
simple
vector
```
</details>



```c
vector(type)                             -> Declares a generic vector of the specified type (macro, no return).
vector_init(vec)                         -> Initializes the vector. Warns if already initialized. (void).
vector_is_valid(vec)                     -> Returns nonzero if the vector is properly initialized (macro, int).
vector_push_back(vec, value)             -> Appends value to the end of the vector, grows if needed. (void, prints error on fail).
vector_push_back_args(vec, ...)          -> Appends multiple values at once. (void, prints error on fail).
vector_at(vec, index)                    -> Returns the element at index. Bounds-checked in debug mode (macro, element type).
vector_size(vec)                         -> Returns number of elements in the vector (macro, size_t).
vector_capacity(vec)                     -> Returns current allocated capacity (macro, size_t).
vector_empty(vec)                        -> Returns 1 if vector is empty, 0 otherwise (macro, int).
vector_back(vec)                         -> Returns the last element (macro, element type).
vector_front(vec)                        -> Returns the first element (macro, element type).
vector_pop_back(vec)                     -> Removes the last element. (void, prints error if not initialized/empty).
vector_clear(vec)                        -> Removes all elements but keeps memory allocated. (void, prints error if not initialized).
vector_destroy(vec)                      -> Frees all memory and marks vector as destroyed. (void, prints error if already destroyed or not initialized).
vector_reserve(vec, new_capacity)        -> Ensures capacity is at least new_capacity. (void, prints error on fail).
vector_resize(vec, new_size, def_val)    -> Changes vector size, fills new elements with default_value. (void, prints error on fail).
vector_shrink_to_fit(vec)                -> Reduces capacity to match size, freeing unused memory. (void, prints error if not initialized).
vector_foreach(vec, item)                -> Macro for iterating over elements; item is a pointer to each element.
vector_find(vec, value)                  -> Returns index of first occurrence of value, or 0 if not found (macro, size_t).
vector_find_index(vec, value)            -> Returns index of first occurrence of value, or 0 if not found (macro, int).
vector_find_custom(vec, value, cmp_func) -> Returns index of first occurrence using custom comparator, or 0 if not found (macro, int).
```
