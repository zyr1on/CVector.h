# CVector.h – Simple & Safe Generic Vector Header Only Library for C 

This header-only library provides a fast, safe, and easy-to-use generic dynamic array (vector) implementation for C. With modern features and robust error checks, you can enjoy the flexibility of C++-style vectors right in C, with zero dependencies and zero runtime overhead for most operations.

---

## Features

-  **Type-safe Macros:** Create vectors of any type with compile-time safety.
-  **Branch Prediction and Fast Growth:** Uses bitwise tricks for efficient resizing.
-  **Debug Checks:** Catches double init/free, out-of-bounds, and misuse.
-  **No Hidden Allocations:** All memory management is explicit and safe.
---

##  Quick Start

```c
// "This header file can be found in include/vector.h."
#include "vector.h" 

vector(T) v;
vector_init(v);
...
...
vector_destroy(v);
```

<details>
<summary> CLICK FOR EXAMPLE USAGE WITH STRUCTS </summary>

```c
#include "vector.h"

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


<details>
<summary> CLICK FOR EXAMPLE USAGE WITH STRUCTS 2 </summary>

```c
#include "vector.h"

struct Vector3{
    float x,y,z;
};
typedef struct Vector3 Vector3;

void vector_print(Vector3* vector) {
    printf("(%f,%f,%f) ", vector->x,vector->y,vector->z);
}


int main(int argc, char const *argv[])
{
    vector(Vector3) vec;
    vector_init(vec);
    
    vector_push_back(vec, ( (Vector3){.x = 0.0, .y = 0.33, .z = 0.15} ) );
    vector_push_back_args(vec, {0.12,0.13,0.14}, {0.31321,0.1321,0.513532} );

    vector_foreach(vec,item) {  // item is pointer to Vector3 struct in vec.data
        vector_print(item);     // if vector_print takes Vector3 vector instead of Vector3* pointer, vector_print should be vector_print(*item);
        printf("\n");
    }

    vector_destroy(vec);

    return 0;
}

}
```
```bash
OUTPUT:
(0.000000,0.330000,0.150000)
(0.120000,0.130000,0.140000)
(0.313210,0.132100,0.513532)
```

</details>

<details>
<summary> CLICK FOR EXAMPLE USAGE WITH STRUCTS via vector_emplace_back </summary>

```c
#include "vector.h"

typedef struct  {
    int value;
    char* data;
} Data;

int main() {
    vector(Data) vec;
    vector_init(vec);
    vector_reserve(vec,5);

    for(int i=0; i < 5; i++)
        vector_emplace_back(vec, i, "hello world"); // for loop

    
    vector_emplace_back(vec, 12,"test"); // ordered,
    vector_emplace_back(vec, .data = "hi", .value = 15); // unordered,

    vector_foreach(vec, item) {
        printf("value: %d, data:%s\n", item->value,item->data);
    }
    vector_destroy(vec);
}

/*
OUTPUT:
value: 0, data:hello world
value: 1, data:hello world
value: 2, data:hello world
value: 3, data:hello world
value: 4, data:hello world
value: 12, data:test
value: 15, data:hi
*/
```
</details>

<details>
<summary> CLICK FOR EXAMPLE USAGE WITH NESTED STRUCTS via vector_emplace_back </summary>

```c
#include "vector.h"

typedef struct {
    float x;
    float y;
}Vector2f;

typedef struct  {
    int flag;
    Vector2f my_vec;
} Data;

int main() {
    vector(Data) vec;
    vector_init(vec);
    vector_reserve(vec,5);

    // .flag = 3, .my_vec.x = 5, .my_vec.y = 9
    vector_emplace_back(vec, 3, 5,9);
    
    // .flag = 5, .my_vec.x = 13, .my_vec.y = 19
    vector_emplace_back(vec, .my_vec.x = 13, .flag = 5, .my_vec.y = 19);

    // flag = 15, .my_vec = {3,9}
    vector_emplace_back(vec, 15, {3,9});

    vector_foreach(vec, item) {
        printf("flag: %d, my_vec.x: %.2f, my_vec.y:%.2f\n", item->flag, item->my_vec.x, item->my_vec.y);
    }
   
    vector_destroy(vec);
}

/*
OUTPUT:
flag: 3, my_vec.x: 5.00, my_vec.y:9.00
flag: 5, my_vec.x: 13.00, my_vec.y:19.00
flag: 15, my_vec.x: 3.00, my_vec.y:9.00
*/
```
</details>

<details>
<summary> CVector in OpenGL </summary>

```c
#include "vector.h"

vector(float) vertices;
vector(unsigned int) indices;

vector_init(vertices);
vector_push_back_args(vertices, -0.5f, -0.5f, 0.0f);
vector_push_back_args(vertices,  0.5f, -0.5f, 0.0f);
vector_push_back_args(vertices,  0.5f,  0.5f, 0.0f);
vector_push_back_args(vertices, -0.5f,  0.5f, 0.0f);

vector_init(indices);
vector_push_back_args(indices, 0, 1, 2, 2, 3, 0);

// Upload to GPU and draw as usual with VAO/VBO/EBO
```
For more detailed code here => [`OpenGLDrawSquareWithVector`](https://github.com/zyr1on/CVector.h/blob/main/src/OpenGLDrawSquareWithVector.c) 

</details>

---

**📖 API Reference DOCUMENTATION**  
 [See the full API documentation here!](API.md)  
Explore all vector macros and functions with examples and detailed descriptions.

---

** FOR MORE  EXAMPLES**  
 [See practical vector usage and code samples here!](EXAMPLES.md)  
Explore real-world examples including 3D vector structs and string arrays with clean iteration and bulk operations.


**DOWNLOAD**  
[Header File!](include/vector.h)  



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

| Function                  | vector.h                        | std::vector                         |
|---------------------------|----------------------------------|-------------------------------------|
| declare vector            | vector(T) vec; ✔️                  | std::vector<T> v; ✔️              |
| init                      | vector_init(vec) ✔️              | automatic ✔️                         |
| data()                      | vec.data ✔️              | vec.data() ✔️                         |
| push_back                 | vector_push_back() ✔️              | v.push_back ✔️                      |
| emplace_back                 | vector_emplace_back() ✔️              | v.emplace_back ✔️                      |
| push_back (variadic)      | vector_push_back_args() ✔️         | (C++20: v.insert w/ fold) ✔️/⚠️        |
| insert                    | vector_insert() ✔️                 | v.insert ✔️                         |
| insert_range              | vector_insert_range() ✔️           | v.insert(begin, arr, arr+count) ✔️ |
| insert_args               | vector_insert_args() ✔️            | (C++20: variadic insert) ✔️/⚠️         |
| at                        | vector_at() ✔️                     | v.at ✔️                             |
| operator[]                | vec.data[i] ✔️              | v[i] ✔️                             |
| size                      | vector_size() or vec.size ✔️                   | v.size() ✔️                         |
| capacity                  | vector_capacity() or vec.capacity ✔️               | v.capacity() ✔️                     |
| empty                     | vector_empty() ✔️                  | v.empty() ✔️                        |
| find                      | vector_find() ✔️                   | std::find (with std::begin) ✔️ (NOT std::vector feat)    |
| find_custom               | vector_find_custom() ✔️            | std::find_if + lambda ✔️ (NOT std::vector feat)          |
| back                      | vector_back() ✔️                   | v.back() ✔️                         |
| front                     | vector_front() ✔️                  | v.front() ✔️                        |
| pop_back                  | vector_pop_back() ✔️               | v.pop_back() ✔️                     |
| clear                     | vector_clear() ✔️                  | v.clear() ✔️                        |
| destroy                   | vector_destroy() ✔️                | automatic ✔️                 |
| reserve                   | vector_reserve() ✔️               | v.reserve() ✔️                     |
| resize                    | vector_resize() ✔️                | v.resize(new_size, def_val) ✔️     |
| shrink_to_fit             | vector_shrink_to_fit() ✔️          | v.shrink_to_fit() ✔️               |
| foreach                   | vector_foreach() ✔️                | range-based for ✔️                 |
| swap                     | vector_swap() ✔️                  | v.swap() ✔️               |


> **Notes:**  
> ✔️ = Has a direct equivalent.  
> ❌ = Not directly available in STL.  
> ⚠️ = Possible with C++20 or workaround.  
