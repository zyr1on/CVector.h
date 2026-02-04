## 🌟 Example Usage


<details>
<summary> <strong>Basic Integer Vector Example (<code>vector(int)</code>)</strong></summary>

```c
#include "vector.h"

int main(void)
{
    // 1. Define and initialize the vector.
    vector(int) vec;
    vector_init(vec);

    // 2. Add elements (one by one and in bulk)
    vector_push_back(vec, 5);
    vector_push_back(vec, 12);
    vector_push_back(vec, 13);
    vector_push_back_args(vec, 7, 24, 25); // Multiple insert

    // 3. Iterate and print elements
    vector_foreach(vec, item) {
        printf("%d ", *item); // 5 12 13 7 24 25
    }
    printf("\n");

    // 4. Get vector size and capacity
    int size = vector_size(vec);
    int capacity = vector_capacity(vec);
    printf("\nvector size: %d | vector_capacity: %d\n", size, capacity);

    // 5. Shrink capacity to fit size exactly
    vector_shrink_to_fit(vec);
    size = vector_size(vec);
    capacity = vector_capacity(vec);
    printf("after shrink to fit:\nvector size: %d | vector_capacity: %d\n", size, capacity);

    // 6. Find index of an element (binary search)
    int index = vector_find(vec, 24); // 4
    printf("binary search of 24 index: %d\n", index);

    // 7. Access a value at a certain index
    int value_at = vector_at(vec, 2); // 13
    printf("vector_at(vec,2) =  %d\n", value_at);

    // 8. Clear the vector and check if it's empty
    vector_clear(vec);
    if(vector_empty(vec))
        printf("vector is empty\n");

    // 9. Clean up memory
    vector_destroy(vec);

    return 0;
}
```

```bash
Output:
5 12 13 7 24 25 

vector size: 6 | vector_capacity: 8

after shrink to fit:
vector size: 6 | vector_capacity: 6

binary search of 24 index: 4

vector_at(vec,2) =  13

vector is empty
```
</details>

---

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

---

<details>
<summary><strong>3D Vector (struct)  Example (<code>vector(vec3)</code>)</strong></summary>

This example demonstrates how to create a vector of 3D points, push elements, iterate, and clean up:

```c
#include "vector.h"

typedef struct {
    float x, y, z;
} vec3;

int main(void) {
    vector(vec3) container;
    vector_init(container); // Initialize before use!

    vec3 a = {3.2, 2.1, 33.59};
    vec3 b = {0.0, 0.0, 0.0};

    vector_push_back(container, a);
    vector_push_back(container, b);

    // Push multiple elements at once (batch insert)
    vector_push_back_args(container, {1,2,3}, {1,2,4}, {1,2,32});

    // Clean and safe iteration with vector_foreach
    vector_foreach(container, p) {
        printf("(%.2f, %.2f, %.2f)\n", p->x, p->y, p->z);
    }

    // Or use classic indexing:
    // for(int i = 0; i < vector_size(container); i++)
    //     printf("(%.2f, %.2f, %.2f)\n", container.data[i].x, container.data[i].y, container.data[i].z);

    vector_destroy(container); // Always destroy to avoid leaks!
    return 0;
}
```
**Output:**
```
(3.20, 2.10, 33.59)
(0.00, 0.00, 0.00)
(1.00, 2.00, 3.00)
(1.00, 2.00, 4.00)
(1.00, 2.00, 32.00)
```
</details>

---

<details>
<summary><strong>3D Vector (struct) with Custom Comparison (<code>vector_find_custom</code>)</strong></summary>

Find the index of a struct using your own comparison function:

```c
#include "vector.h"

typedef struct {
    float x, y, z;
} vec3;

int compare_func(vec3 a, vec3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

void printVec3(vec3 a) {
    printf("(%.2f, %.2f, %.2f): ", a.x, a.y, a.z);
}

int main(void) {
    vector(vec3) container;
    vector_init(container);

    vec3 a = {3.2, 2.1, 33.59}, b = {0,0,0}, c = {1.2,3.4,5.6}, d = {33.1,96.28,-45.6};
    vector_push_back(container, a);
    vector_push_back(container, b);
    vector_push_back_args(container, c, d, {11.4,12.64,-45635.12});

    int idx = vector_find_custom(container, ((vec3){1.2, 3.4, 5.6}), compare_func);

    printVec3((vec3){1.2, 3.4, 5.6});
    printf("linear search index: %d\n", idx);

    vector_destroy(container);
    return 0;
}
```
**Output:**
```
(1.20, 3.40, 5.60): linear search index: 2
```
</details>

---

<details>
<summary><strong> vector_emplace_back example with Struct (<code>vector_emplace_back(vec,value,value,...)</code>)</strong></summary>

This example demonstrates how to create a vector of 3D points, push elements, iterate, and clean up:

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
```
**Output:**
```
value: 0, data:hello world
value: 1, data:hello world
value: 2, data:hello world
value: 3, data:hello world
value: 4, data:hello world
value: 12, data:test
value: 15, data:hi
```
</details>

---

<details>
<summary><strong> vector_emplace_back example with Nested Structs (<code>vector_emplace_back(vec,value,value,...)</code>)</strong></summary>

This example demonstrates how to create a vector of 3D points, push elements, iterate, and clean up:

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

```
**Output:**
```
flag: 3, my_vec.x: 5.00, my_vec.y:9.00
flag: 5, my_vec.x: 13.00, my_vec.y:19.00
flag: 15, my_vec.x: 3.00, my_vec.y:9.00
```
</details>

---


<details>
<summary><strong>String Vector Example (<code>vector(char*)</code>)</strong></summary>

This shows how to work with a vector of C-strings:

```c
#include "vector.h"

typedef const char* string;

int main(void) {
    vector(string) vec;
    vector_init(vec);

    vector_push_back(vec, "hello");
    vector_push_back(vec, "world");
    vector_push_back_args(vec, "just", "simple", "vector");

    vector_foreach(vec, item)
        printf("%s\n", *item);

    vector_destroy(vec);
    return 0;
}
```
**Output:**
```
hello
world
just
simple
vector
```
</details>

---

<details>
<summary><strong>String Vector Example With strcmp Compare Function(<code>vector(char*)</code>)</strong></summary>

This shows how to work with a vector of C-strings:

```c
#include "vector.h"
#include<string.h> // for strcmp


typedef const char *string;
int str_eq(string a, string b) { // function to compare string and returns true or false with int value.
    return strcmp(a, b) == 0;
}

int main(void) {
    vector(string) vec;
    vector_init(vec);

    vector_push_back(vec, "hello");
    vector_push_back(vec, "world");
    vector_push_back_args(vec, "just", "simple", "vector");

    vector_foreach(vec, item)
        printf("%s, ", *item);
    printf("\n\n");

    int index = vector_find_custom(vec, "just", str_eq); // using str_eq function to compare str_eq("just", vector_contents)
    printf("'just' index: %d\n",index);
    vector_destroy(vec);
    return 0;
}
```
**Output:**
```
hello, world, just, simple, vector, 

'just' index: 2
```
</details>

---

<details>
<summary><strong>insert range of elements (Point structure) (<code>vector(char*)</code>)</strong></summary>

```c
#include "vector.h"
typedef struct {
    int x;
    int y;
} Point;

int main(void) {
    vector(Point) vec;
    vector_init(vec);
   
    Point a = {1,2};
    vector_push_back(vec,a); 
    vector_push_back(vec, ( (Point) {.x = 12, .y = 34}) );
    vector_push_back(vec, ( (Point) {.x = 45, .y = 56}) ); 
    
    vector_foreach(vec, item)
        printf("(%d, %d)\n", item->x, item->y);
    
    printf("---\n");

    Point points[] = {{.x = 3, .y =5} , {.x = -31, .y = -52}}; 
    vector_insert_range(vec, 1, points, 2); // inserts elements to index 1
    
    vector_foreach(vec, item)
        printf("(%d, %d)\n", item->x, item->y);

    vector_destroy(vec);
    return 0;
}
```
**Output:**
```
(1, 2)  
(12, 34)
(45, 56)
---     
(1, 2)
(3, 5)
(-31, -52)
(12, 34)
(45, 56)
```
</details>

---

<details>
<summary><strong>insert args of elements (Point structure) (<code>vector(char*)</code>)</strong></summary>

```c
#include "vector.h"
typedef struct {
    int x;
    int y;
} Point;

int main(void) {
    vector(Point) vec;
    vector_init(vec);
   
    vector_push_back(vec, ( (Point) {.x = 12, .y = 34}) );
    vector_push_back(vec, ( (Point) {.x = 45, .y = 56}) ); 
    
    vector_foreach(vec, item)
        printf("(%d, %d)\n", item->x, item->y);
    
    printf("\n---\n");

    vector_insert_args(vec, 1 , {1,2}, {3,4});

    vector_foreach(vec, item)
        printf("(%d, %d)\n", item->x, item->y);

    vector_destroy(vec);
    return 0;
}
```
**Output:**
```
(12, 34)
(45, 56)
---
(12, 34)
(1, 2)
(3, 4)
(45, 56)
```
</details>

---

**Tips:**  
- Use `vector_push_back_args` for convenient batch insertion.  
- `vector_foreach` gives you cleaner and safer iteration.
- Always call `vector_init` when need to create a vector. 
- Always call `vector_destroy` when done to avoid memory leaks.
