# Vision và mục tiêu Version 2

## 1. Giữ lại những gì v1 làm đúng

Không rewrite:

- static-first object ownership;
- fixed-priority scheduler baseline;
- intrusive ready/wait/timeout structures;
- direct handoff IPC;
- opaque public objects;
- kernel/haievent separation;
- target manifest;
- one-task-per-AO default model;
- host-testable generic C.

## 2. Mục tiêu 2.0

### Correctness

- interrupt priority contract có thể kiểm tra;
- task-return/AO-internal failure đi qua controlled diagnostics;
- run-to-completion misuse có debug detection;
- HSM transition semantics có model-based tests;
- stronger ownership/trace diagnostics.

### Portability

- ít nhất target thứ hai chạy thật;
- target schema không duplicate vô tội vạ;
- examples target-specific được capability-gate/adapt;
- architecture capability rõ FPU/MPU/interrupt ceiling/tickless.

### Event modeling

- hierarchical states;
- parent event propagation;
- initial substates;
- LCA-based transitions;
- deferred event/recall;
- optional history sau core ổn định.

### Power/time

- tickless idle;
- next-deadline contract;
- optional extended uptime diagnostics.

### Observability

- fixed-size trace ring;
- task/IPC/timer/AO events;
- build/version identity trong panic record;
- export qua debugger/UART adapter.

## 3. Non-goals 2.0

Không đưa vào critical path 2.0:

- SMP;
- general POSIX layer;
- filesystem/network stack;
- general dynamic kernel heap;
- full vendor-independent HAL;
- automatic deadlock prevention;
- safety certification;
- shared-executor AO production default;
- every possible HSM feature.

## 4. Chỉ số thành công

Version 2 tốt hơn nếu:

- target mới thêm mà không sửa kernel;
- interrupt latency giảm/được kiểm soát;
- debug fault có timeline;
- state model phức tạp viết ít duplicated transition code hơn;
- v1 application migration predictable;
- test matrix bắt regression trước hardware.

## 5. Scope discipline

Mỗi feature proposal phải có:

```text
problem
contract
memory cost
runtime cost
ISR implications
failure modes
tests
migration
documentation
```

Không merge header/API trước implementation và tests.
