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

---


**📖 API Reference DOCUMENTATION**  
👉 [See the full API documentation here!](API.md)  
Explore all vector macros and functions with examples and detailed descriptions.

---

**🧑‍💻 Usage EXAMPLES**  
👉 [See practical vector usage and code samples here!](EXAMPLES.md)  
Explore real-world examples including 3D vector structs and string arrays with clean iteration and bulk operations.


**🧑‍💻 DOWNLOAD**  
👉 [Header File!](src/CVector.h)  



# Benchmark Results: CVector vs std::vector    |   push_back()

This benchmark compares the performance of a custom C vector implementation (`CVector`) against C++'s standard library vector (`std::vector`) when pushing back 1,000,000 integer elements. The test was repeated 5 times for each vector type, and the durations were measured using `std::chrono`. Below is a summary of the results:

| Run   | CVector (ms) | std::vector (ms) |
|-------|--------------|------------------|
| 1     | 2.0348       | 5.9383           |
| 2     | 1.9931       | 6.9766           |
| 3     | 2.9904       | 5.9795           |
| 4     | 1.9930       | 5.9806           |
| 5     | 1.9932       | 5.9800           |
| **Average** | **2.2009** | **6.1710**         |

---

- **Test Method**: Both vectors were tested by sequentially adding 1,000,000 integers using their respective `push_back` methods. Each test was repeated 5 times to reduce measurement noise.


# Benchmark Results: CVector vs std::vector middle pos     |   insert()

This benchmark measures the performance of inserting elements into the **middle** of a vector for both a custom C vector implementation (`CVector`) and C++'s standard library vector (`std::vector`). Each vector had 100,000 elements inserted one-by-one into the middle, and the test was repeated 5 times for each implementation. Times are reported in milliseconds (ms).

| Run   | CVector (ms) | std::vector (ms) |
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

- **CPU:** Intel Core i3-12100f @ 2.7 GHz
- **RAM:** 16 GB DDR4
- **OS:** Windows 10 22H2
- **Compiler:** mingw g++

Benchmark results may vary on different platforms.
