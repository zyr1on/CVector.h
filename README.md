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

##  Example with "int"

```c
#include <stdio.h>
#include "vector.h"

int main() {
    // 1. Define a vector of integers
    vector(int) numbers;

    // 2. Initialize (Required before use)
    vector_init(numbers);

    // 3. Add elements
    vector_push_back(numbers, 10);
    vector_push_back(numbers, 20);

    // 4. Access elements
    // You can access vector data directly using numbers.data[index]
    printf("First element: %d\n", numbers.data[0]); 

    // 5. Clean up memory
    vector_destroy(numbers);
    
    return 0;
}
```
```
OUTPUT:
First element: 10
```

##  ⚠️ Important: Passing Vectors to Functions
Since vector(T) expands to an anonymous struct, you cannot use it directly in function parameters (it will cause a type mismatch error).

You must use typedef to name your vector type before passing it to functions.

❌ Incorrect
```c
// This will NOT compile because the compiler treats these as two different types
void process_data(vector(int) *vec) { ... } 

int main() {
    vector(int) v; // Type mismatch with the function above
    process_data(&v);
}
```
✅ Correct Usage
```c
// 1. Create a named type
typedef vector(int) IntVec;

// 2. Use the named type in the function parameter (pass by pointer)
void process_data(IntVec *vec) {
    vector_push_back(*vec, 42);
}

int main() {
    IntVec v;          // Use the named type
    vector_init(v);
    
    process_data(&v);  // Pass by reference
    
    vector_destroy(v);
    return 0;
}
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
```
```bash
OUTPUT:
(0.000000,0.330000,0.150000)
(0.120000,0.130000,0.140000)
(0.313210,0.132100,0.513532)
```

</details>

<details>
<summary><strong>⚠️ CLICK FOR AN EXAMPLE OF PASSING VECTORS TO FUNCTIONS</strong></summary>

```c
#include <stdio.h>
#include "vector.h"

// Since vector(int) expands to an anonymous struct (struct { ... }), 
// declaring it directly in multiple places would create incompatible types.
// We use typedef to define a single, shared type that can be passed to functions.
typedef vector(int) vec_int;

// Standard recursive helper function to calculate factorial.
int factorial(int x) {
    if(x < 1)
        return 1;
    return x * factorial(x - 1);
}

// Accepts the vector by pointer so we can modify the original instance.
void addFactorial(vec_int* vec, int x) {
    // We must dereference the pointer (*vec) because the macro 
    // expects the struct itself, not a pointer to it.
    vector_emplace_back(*vec, factorial(x));
}

int main() {
    // Instantiate two separate vectors using the defined type.
    vec_int vec1;
    vec_int vec2;

    // Initialize internal pointers and metadata.
    vector_init(vec1);
    vector_init(vec2);
    
    // Modify vectors via the helper function.
    addFactorial(&vec1, 4); // Adds 24
    addFactorial(&vec1, 5); // Adds 120
    
    addFactorial(&vec2, 6); // Adds 720
    addFactorial(&vec2, 7); // Adds 5040

    // Iterate and print elements using the foreach macro.
    printf("vec1: "); vector_foreach(vec1, item) printf("%d, ", *item);
    printf("\nvec2: "); vector_foreach(vec2, item) printf("%d, ", *item);
    
    // Clean up allocated memory.
    vector_destroy(vec1);
    vector_destroy(vec2);

    return 0;
}
```
```bash
OUTPUT:
vec1: 24, 120,
vec2: 720, 5040,
```

</details>

<details>
<summary><strong>⚠️ CLICK FOR AN EXAMPLE OF PASSING VECTORS TO FUNCTIONS 2</strong></summary>

```c
#include <stdio.h>
#include <math.h>
#include "vector.h"

// 1. Rename struct for clarity
// 'Vector2' is a standard name in graphics programming (x, y).
typedef struct {
    float x;
    float y;
} Vector2;

// 2. CRITICAL STEP: Typedef for the Vector
// Why? The macro 'vector(Vector2)' creates an "anonymous struct". 
// Without typedef, C considers 'vector(Vector2)' in main() and 'vector(Vector2)' 
// in a function parameter as two DIFFERENT types. 
// Giving it a name (VectorList) ensures type compatibility everywhere.
typedef vector(Vector2) VectorList;

/**
* Normalizes all vectors in the list.
* Normalization means keeping the direction but changing the length (magnitude) to 1.0.
* @param list Pointer to the vector container
*/
void normalize_all_vectors(VectorList* list) {
    // Renamed 'item' to 'current_vec' for clarity
    vector_foreach(*list, current_vec) {
        
        // Calculate Magnitude (Hypotenuse) using Pythagorean theorem
        // Formula: ||v|| = sqrt(x^2 + y^2)
        float magnitude = sqrtf((current_vec->x * current_vec->x) + (current_vec->y * current_vec->y));

        // Safety Check: Prevent "Division by Zero"
        // If the vector is (0,0), magnitude is 0. Dividing by 0 crashes the program.
        // We check against a small epsilon (0.0001f) because float comparison is imprecise.
        if (magnitude > 0.0001f) { 
            current_vec->x /= magnitude; // x = x / magnitude
            current_vec->y /= magnitude; // y = y / magnitude
        }
    }
}

// Prints the coordinates of all vectors in the list.
void print_vectors(VectorList* list) {
    vector_foreach(*list, current_vec) {
        // %.2f limits the output to 2 decimal places (e.g., 0.60)
        printf("(%.2f, %.2f) \n", current_vec->x, current_vec->y);
    }
    printf("---------------------------\n");
}

int main() {
    // Initialize the named vector type
    VectorList my_vectors;
    vector_init(my_vectors);

    // --- Adding Data ---
    // Using 'emplace_back' constructs the object directly in the vector.
    
    // 1. Standard approach (3-4-5 Triangle)
    vector_emplace_back(my_vectors, 3.0f, 4.0f);
    
    // 2. Valid in C, but causes a compilation error in C++20
    // Error: designator order for field 'x' does not match declaration order

    vector_emplace_back(my_vectors, .x = 15.0f, .y = 14.3f);
    vector_emplace_back(my_vectors, .y = 28.0f, .x = 16.0f); // Always strictly follow the struct definition order (x then y), but it works on C
    vector_emplace_back(my_vectors, .x = 7.0f,  .y = 24.0f); // 7-24-25 Triangle

    // Print original values
    printf("Original Values:\n");
    print_vectors(&my_vectors);

    // Perform normalization (Pass by reference/pointer)
    normalize_all_vectors(&my_vectors);
    
    // Print normalized values
    printf("Normalized Values (Unit Vectors):\n");
    print_vectors(&my_vectors);
    
    // Clean up memory
    vector_destroy(my_vectors);
    
    return 0;
}
```
```bash
OUTPUT:
Original Values:
(3.00, 4.00)
(15.00, 14.30)
(16.00, 28.00)
(7.00, 24.00)
---------------------------
Normalized Values (Unit Vectors):
(0.60, 0.80)
(0.72, 0.69)
(0.50, 0.87)
(0.28, 0.96)
---------------------------
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


# **📖 API Reference DOCUMENTATION**  
### [See the full API documentation here!](API.md)  
Explore all vector macros and functions with examples and detailed descriptions.


## **FOR MORE  EXAMPLES**  
### [See practical vector usage and code samples here!](EXAMPLES.md)  
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
| declare vector            | vector(T) vec; ✔️                  | "std::vector<<T>> v;" ✔️              |
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
