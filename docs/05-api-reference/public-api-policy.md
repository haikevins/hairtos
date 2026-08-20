# Public API policy

> **Scope:** Quy định boundary nào application được phép phụ thuộc trong `hairtos 1.0.0-rc1`.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](mutex-api.md) · [Next →](queue-api.md)

## Mục lục

- [Public surface](#public)
- [Internal surface](#internal)
- [Opaque storage](#opaque)
- [Compatibility](#compat)
- [Testing boundary](#testing)
- [References](#references)

<a id="public"></a>
## Public surface

Application bình thường dùng:

```c
#include "hairtos/hairtos.h"
#include "haievent/haievent.h"
#include "board.h"
```

Các header con trong `kernel/include/hairtos/` và `haievent/include/haievent/` cũng là public khi include trực tiếp có chủ đích.

<a id="internal"></a>
## Internal surface

`kernel/internal/` và `haievent/internal/` **không phải API compatibility surface**. Chúng chứa TCB/control block, intrusive nodes, scheduler/wait/timeout internals và helper dùng để test hoặc module core phối hợp. CMake chỉ cấp internal include cho target/tests/example có lý do cụ thể.

<a id="opaque"></a>
## Opaque storage

Public type như `hr_task_t` là union `max_align_t + unsigned char storage[N]`. Ưu điểm:

- caller cấp phát tĩnh mà không biết internal layout;
- internal struct có thể thay đổi nếu vẫn fit config storage;
- compile-time `_Static_assert` bắt storage quá nhỏ;
- assembly chỉ phụ thuộc contract tối thiểu (ví dụ saved SP ở TCB offset 0), không cần expose TCB cho application.

<a id="compat"></a>
## Compatibility

- Public function signature/status semantics là thứ cần migration policy khi thay đổi.
- Internal struct size/order không có compatibility promise.
- Config macro là build-time contract; thay giá trị có thể đổi RAM footprint/timing và phải rebuild toàn bộ binary.
- `docs/09-version2` không reserve API v2 bằng cách làm application phụ thuộc symbol chưa implemented.

<a id="testing"></a>
## Testing boundary

Host tests được phép include internal header để unit-test data structure/policy. Điều đó không biến internal header thành public API. Example 15 cũng được CMake cấp internal access cho benchmark có chủ đích.

<a id="references"></a>
## References


**Nguồn implementation trong repository:**
- `kernel/include/hairtos/`
- `kernel/internal/`
- `haievent/include/haievent/`
- `haievent/internal/`
- `CMakeLists.txt`
- `cmake/hairtos_examples.cmake`
