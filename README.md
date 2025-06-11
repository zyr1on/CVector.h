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

## 🌟 Example Usage

<details>
<summary><strong>3D Vector Example <code>vector(vec3)</code></strong></summary>

```c
#include "CVector.h"

typedef struct {
    float x;
    float y;
    float z;
} vec3;

int main(void)
{
    vector(vec3) container;
    vector_init(container); // Always initialize your vector before use!

    vec3 a = {3.2, 2.1, 33.59};
    vec3 b = {0.0, 0.0, 0.0};

    vector_push_back(container, a);
    vector_push_back(container, b);

    // You can push multiple elements at once:
    vector_push_back_args(container, {1,2,3}, {1,2,4}, {1,2,32});

    // Elegant iteration:
    vector_foreach(container, p){
        printf("(%.2f, %.2f, %.2f)\n", p->x, p->y, p->z);
    }

    /* Or use traditional indexing if you prefer:
    for(int i = 0; i < vector_size(container); i++) {
        printf("(%.2f, %.2f, %.2f)\n", container.data[i].x, container.data[i].y, container.data[i].z);
    }
    */

    vector_destroy(container); // Don't forget to destroy the vector to avoid memory leaks!
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

---


<details>
<summary><strong>3D Vector Example with custom compare function <code>vector(char*)</code></strong></summary>

```c
#include "CVector.h"

typedef struct {
    float x;
    float y;
    float z;
} vec3;

int compare_func(vec3 a, vec3 b) {
    return a.x == b.x && a.y == b.y &&  a.z == b.z;
}

void printVec3(vec3 a) {
    printf("(%.2f, %.2f, %.2f): ",a.x,a.y,a.z);
}

int main(void)
{
    vector(vec3) container;
    vector_init(container); 

    vec3 a = {3.2, 2.1, 33.59};
    vec3 b = {0.0, 0.0, 0.0};
    vec3 c = {1.2, 3.4, 5.6};
    vec3 d = {33.1, 96.28, -45.6};
    

    vector_push_back(container, a);
    vector_push_back(container, b);
    vector_push_back_args(container, c,d);

    int index = vector_find_custom(container, ((vec3){1.2, 3.4, 5.6}), compare_func); // 2
    // int index = vector_find_custom(container, c, compare_func); -> 2
    
    printVec3(((vec3){1.2, 3.4, 5.6}));
    printf("index at: %d\n", index);

    vector_destroy(container); // Don't forget to destroy the vector to avoid memory leaks!
    return 0;
}
```
```bash
OUTPUT:
(1.20, 3.40, 5.60): index at: 2
```

</details>

---


<details>
<summary><strong>String Vector Example <code>vector(char*)</code></strong></summary>

```c
#include "CVector.h"

typedef char* string;

int main(void)
{
    vector(string) vec;
    vector_init(vec);

    vector_push_back(vec, "hello");
    vector_push_back(vec, "world");

    // Add multiple strings at once:
    vector_push_back_args(vec, "just", "simple", "vector");

    vector_foreach(vec, item)
        printf("%s\n", *item);

    vector_destroy(vec);
    return 0;
}
```

```bash
OUTPUT:
hello
world
just
simple
vector
```
</details>

---


✨ **Tip:**  
- Use `vector_push_back_args` for convenient bulk insertion.
- The `vector_foreach` macro makes your code cleaner and safer.
- Always call `vector_destroy` when you're done!

**📖 API Reference**  
👉 [See the full API documentation here!](API.md)  
Explore all vector macros and functions with examples and detailed descriptions.
