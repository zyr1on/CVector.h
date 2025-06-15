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

vector(T) v;
vector_init(v);
...
...
vector_destroy(v);
```

<details>
<summary> CLICK FOR EXAMPLE USAGE WITH STRUCTS </summary>

```c
#include "CVector.h"

typedef struct {
    int id;
    int age;
    const char* name;
}Students;

int main(int argc, char const *argv[])
{
    vector(Students) vec;
    vector_init(vec); // !!! ALWAYS INITIALIZE VECTOR

    // classic push_back
    vector_push_back(vec, ((Students){.id = 1, .age = 22, .name = "semih ozdemir"}));

    // variadic function
    vector_push_back_args(vec, {4, 89, "bergman"}, {3, 88, "kurosowa"},  {2, 54, "tarkovski"}, {5, 70, "kubrick"});
    
    vector_foreach(vec, item) // item is pointer
        printf("id: %d age: %d name: %s\n", item->id, item->age, item->name);
    printf("\n");

    int size = vector_size(vec);
    int capacity = vector_capacity(vec);

    printf("vector size: %d vector capacity: %d\n", size,capacity);
    printf("...shrint to fit...\n");
    
    vector_shrink_to_fit(vec);
    size = vector_size(vec);
    capacity = vector_capacity(vec);
    printf("vector size: %d vector capacity: %d\n", size,capacity);
    
    vector_destroy(vec); // !!!! IMPORTANT: dont forget to destory vector to avoid memory leaks    
    return 0;
}
```
```bash
OUTPUT:
id: 1 age: 22 name: semih ozdemir
id: 4 age: 89 name: bergman
id: 3 age: 88 name: kurosowa
id: 2 age: 54 name: tarkovski
id: 5 age: 70 name: kubrick

vector size: 5 vector capacity: 8
...shrint to fit...
vector size: 5 vector capacity: 5
```

</details>

---

**📖 API Reference DOCUMENTATION**  
👉 [See the full API documentation here!](API.md)  
Explore all vector macros and functions with examples and detailed descriptions.

---

**🧑‍💻 FOR MORE  EXAMPLES**  
👉 [See practical vector usage and code samples here!](EXAMPLES.md)  
Explore real-world examples including 3D vector structs and string arrays with clean iteration and bulk operations.


**🧑‍💻 DOWNLOAD**  
👉 [Header File!](src/CVector.h)  



# Benchmark Results: CVector vs std::vector  

This benchmark compares the performance of a custom C vector implementation (`CVector`) against C++'s standard library vector (`std::vector`) when pushing back 1,000,000 integer elements. The test was repeated 5 times for each vector type, and the durations were measured using `std::chrono`. Below is a summary of the results:

| push_back()   | CVector (ms) | std::vector (ms) |
|-------|--------------|------------------|
| 1     | 2.0348       | 5.9383           |
| 2     | 1.9931       | 6.9766           |
| 3     | 2.9904       | 5.9795           |
| 4     | 1.9930       | 5.9806           |
| 5     | 1.9932       | 5.9800           |
| **Average** | **2.2009** | **6.1710**         |

---

- **Test Method**: Both vectors were tested by sequentially adding 1,000,000 integers using their respective `push_back` methods. Each test was repeated 5 times to reduce measurement noise.


# Benchmark Results: CVector vs std::vector middle pos

This benchmark measures the performance of inserting elements into the **middle** of a vector for both a custom C vector implementation (`CVector`) and C++'s standard library vector (`std::vector`). Each vector had 100,000 elements inserted one-by-one into the middle, and the test was repeated 5 times for each implementation. Times are reported in milliseconds (ms).

| insert()   | CVector (ms) | std::vector (ms) |
|-------|--------------|------------------|
| 1     | 140.530      | 143.115          |
| 2     | 135.547      | 137.540          |
| 3     | 136.541      | 136.543          |
| 4     | 136.543      | 137.540          |
| 5     | 139.533      | 140.748          |
| **Average** | **137.739** | **139.097**         |

- **Test Method:** Each vector starts empty. For each of 100,000 iterations, an element is inserted at the current middle position (`size / 2`).

---

> **Note:** Timings can be affected by system hardware, compiler, and OS. Always report your benchmark environment for reproducibility.


## System Information

- **CPU:** Intel Core i3-12100f @ 3.30 GHz 12M Cache, up to 4.30 GHz
- **RAM:** 16 GB DDR4
- **OS:** Windows 10 22H2
- **Compiler:** mingw gcc ( for g++ use `-fpermissive`)

Benchmark results may vary on different platforms.

# COMPARE WITH C++ STL `std::vector<T> v;`

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
