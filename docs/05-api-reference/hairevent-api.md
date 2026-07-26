# HairEvent API

## 1. Header

```c
#include "hairevent/hairevent.h"
```

## 2. Event pool/event

- `he_event_pool_init`, query counts.
- `he_event_new` cho dynamic event.
- `he_event_init_static` cho static event.
- retain/release và validation queries.

## 3. State machine

- init/start/dispatch.
- `he_state_transition()` trong handler.
- query current handler và context.

## 4. Active Object

- static create với queue/stack/priority.
- post từ task hoặc ISR.
- query name/pending/task/state machine.

## 5. Time event

- create, arm, disarm, rearm, change period.
- query armed và dropped count.

## 6. Publish/Subscribe

- init table.
- subscribe/unsubscribe.
- query subscriber count.
- publish và lấy delivered count.

## 7. Signal rules

Signal framework 0–4; application signal bắt đầu từ `HE_SIG_USER`.

## 8. Ownership rules

Dynamic event phải được release đúng reference count. Không giữ static time-event pointer ngoài thời gian dispatch. Publish consumes publisher dynamic reference theo contract trong header.

## 9. Lưu ý

HairEvent yêu cầu queue, task và software timer tương ứng được link/configure.
