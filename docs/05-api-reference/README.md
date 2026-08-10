# 05 — Public API reference

Application nên include:

```c
#include "hairtos/hairtos.h"
```

và khi dùng event framework:

```c
#include "haievent/haievent.h"
```

## Tài liệu

- [status-and-types.md](status-and-types.md)
- [kernel-and-task-api.md](kernel-and-task-api.md)
- [time-and-context-api.md](time-and-context-api.md)
- [queue-api.md](queue-api.md)
- [semaphore-api.md](semaphore-api.md)
- [mutex-api.md](mutex-api.md)
- [timer-api.md](timer-api.md)
- [diagnostics-and-hooks-api.md](diagnostics-and-hooks-api.md)
- [haievent-api.md](haievent-api.md)
- [public-api-policy.md](public-api-policy.md)

## Context rule

Mỗi API phải được hiểu theo context:

```text
before kernel
task context
ISR context
timer-service callback
AO state handler
```

Không suy luận rằng một API "thread-safe" thì tự động hợp lệ trong ISR.
