# Public API policy

> **Scope:** Defines which boundaries application code may depend on in `hairtos 1.0.0-rc1`.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](mutex-api.md) · [Next →](queue-api.md)

## Table of Contents

- [Public surface](#public)
- [Internal surface](#internal)
- [Opaque storage](#opaque)
- [Compatibility](#compat)
- [Testing boundary](#testing)
- [References](#references)

<a id="public"></a>
## Public surface

Normal applications use:

```c
#include "hairtos/hairtos.h"
#include "haievent/haievent.h"
#include "board.h"
```

Subheaders under `kernel/include/hairtos/` and `haievent/include/haievent/` are also public when intentionally included directly.

<a id="internal"></a>
## Internal surface

`kernel/internal/` and `haievent/internal/` are **not part of the API compatibility surface**. They contain TCB/control blocks, intrusive nodes, scheduler/wait/timeout internals, and helpers used by tests or cooperating core modules. CMake grants internal includes only to targets/tests/examples with a specific reason.

<a id="opaque"></a>
## Opaque storage

Public types such as `hr_task_t` are unions of `max_align_t + unsigned char storage[N]`. Advantages:

- callers allocate objects statically without knowing the internal layout;
- internal structures may change as long as they still fit the configured storage;
- compile-time `_Static_assert` catches undersized storage;
- assembly depends only on the minimum contract (for example, saved SP at TCB offset 0) and does not require exposing the TCB to applications.

<a id="compat"></a>
## Compatibility

- Public function signatures and status semantics require a migration policy when changed.
- Internal structure size/order carries no compatibility promise.
- Configuration macros are build-time contracts; changing them may alter RAM footprint/timing and requires rebuilding the complete binary.
- `docs/09-version2` does not reserve v2 APIs by making applications depend on symbols that are not implemented.

<a id="testing"></a>
## Testing boundary

Host tests may include internal headers to unit-test data structures/policies. That does not make those headers public API. Example 15 also receives intentional internal access from CMake for benchmarking.

<a id="references"></a>
## References


**Implementation sources in the repository:**
- `kernel/include/hairtos/`
- `kernel/internal/`
- `haievent/include/haievent/`
- `haievent/internal/`
- `CMakeLists.txt`
- `cmake/hairtos_examples.cmake`
