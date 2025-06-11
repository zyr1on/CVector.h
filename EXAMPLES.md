## 🌟 Example Usage


<details>
<summary>📝 <strong>Basic Integer Vector Example (<code>vector(int)</code>)</strong></summary>

```c
#include "CVector.h"

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
<summary>🚀 <strong>3D Vector (struct)  Example (<code>vector(vec3)</code>)</strong></summary>

This example demonstrates how to create a vector of 3D points, push elements, iterate, and clean up:

```c
#include "CVector.h"

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
<summary>🔍 <strong>3D Vector (struct) with Custom Comparison (<code>vector_find_custom</code>)</strong></summary>

Find the index of a struct using your own comparison function:

```c
#include "CVector.h"

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
    vector_push_back_args(container, c, d);

    int idx = vector_find_custom(container, ((vec3){1.2, 3.4, 5.6}), compare_func);

    printVec3((vec3){1.2, 3.4, 5.6});
    printf("index at: %d\n", idx);

    vector_destroy(container);
    return 0;
}
```
**Output:**
```
(1.20, 3.40, 5.60): index at: 2
```
</details>

---

<details>
<summary>📝 <strong>String Vector Example (<code>vector(char*)</code>)</strong></summary>

This shows how to work with a vector of C-strings:

```c
#include "CVector.h"

typedef const char *string;

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
<summary>📝 <strong>String Vector Example With strcmp Compare Function(<code>vector(char*)</code>)</strong></summary>

This shows how to work with a vector of C-strings:

```c
#include "CVector.h"
#include<string.h> // for strcmp


typedef const char *string;
int str_eq(string a, string b) {
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

    int index = vector_find_custom(vec, "just", str_eq);
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


✨ **Tips:**  
- Use `vector_push_back_args` for convenient batch insertion.  
- `vector_foreach` gives you cleaner and safer iteration.  
- Always call `vector_destroy` when done to avoid memory leaks.
