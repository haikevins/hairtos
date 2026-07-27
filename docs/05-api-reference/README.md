# 05 — Tham chiếu API

## 1. Mục tiêu

Nhóm này mô tả public API của kernel và `haievent`. Internal header không phải contract cho application.

## 2. Header tổng hợp

```c
#include "hairtos/hairtos.h"
#include "haievent/haievent.h"
```

## 3. Nội dung

- [status-and-types.md](status-and-types.md)
- [kernel-and-task-api.md](kernel-and-task-api.md)
- [time-and-context-api.md](time-and-context-api.md)
- [queue-api.md](queue-api.md)
- [semaphore-api.md](semaphore-api.md)
- [mutex-api.md](mutex-api.md)
- [timer-api.md](timer-api.md)
- [diagnostics-and-hooks-api.md](diagnostics-and-hooks-api.md)
- [haievent-api.md](haievent-api.md)

## 4. Portability

Application nên chỉ dựa trên các API này và `board.h`. Architecture/SoC-specific pin hoặc peripheral identifiers chỉ xuất hiện trong board/driver configuration.

## 5. Kiểm tra

Khi đổi signature public, cần cập nhật header, implementation, host tests, example và tài liệu API trong cùng thay đổi.
