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

... 
...
...

vector_destroy(v);
```

---



✨ **Tip:**  
- Use `vector_push_back_args` for convenient bulk insertion.
- The `vector_foreach` macro makes your code cleaner and safer.
- Always call `vector_destroy` when you're done!

**📖 API Reference**  
👉 [See the full API documentation here!](API.md)  
Explore all vector macros and functions with examples and detailed descriptions.

---

**🧑‍💻 Usage Examples**  
👉 [See practical vector usage and code samples here!](EXAMPLES.md)  
Explore real-world examples including 3D vector structs and string arrays with clean iteration and bulk operations.


**🧑‍💻 DOWNLOAD**  
👉 [Header File!](src/CVector.h)  
