# Example index

> 22 example directories tạo một progression có chủ đích từ bare-metal tới diagnostics/stress.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [Next →](memory-allocator-lab.md)

| Example | Trọng tâm | Environment |
| --- | --- | --- |
| [`01-baremetal-foundation`](../../examples/01-baremetal-foundation/README.md) | `01-baremetal-foundation` — Nền tảng bare-metal | Target |
| [`02-kernel-data-structures-host`](../../examples/02-kernel-data-structures-host/README.md) | `02-kernel-data-structures-host` — Cấu trúc dữ liệu kernel — Demo trên host | Host only |
| [`03-static-task-stack`](../../examples/03-static-task-stack/README.md) | `03-static-task-stack` — TCB tĩnh và ngăn xếp khởi tạo của tác vụ | Target |
| [`04-start-first-task`](../../examples/04-start-first-task/README.md) | `04-start-first-task` — Khởi chạy tác vụ đầu tiên bằng SVC | Target |
| [`05-cooperative-context-switch`](../../examples/05-cooperative-context-switch/README.md) | `05-cooperative-context-switch` — Chuyển ngữ cảnh hợp tác | Target |
| [`06-priority-scheduler`](../../examples/06-priority-scheduler/README.md) | `06-priority-scheduler` — Bộ lập lịch ưu tiên cố định | Target |
| [`07-task-delay-timeout`](../../examples/07-task-delay-timeout/README.md) | `07-task-delay-timeout` — SysTick, trì hoãn tác vụ và timeout | Target |
| [`08-preemption-round-robin`](../../examples/08-preemption-round-robin/README.md) | `08-preemption-round-robin` — Chiếm quyền và Round-Robin | Target |
| [`09-queue-blocking-ipc`](../../examples/09-queue-blocking-ipc/README.md) | `09-queue-blocking-ipc` — Queue và IPC chặn | Target |
| [`10-01-semaphore-from-isr`](../../examples/10-01-semaphore-from-isr/README.md) | `10-01-semaphore-from-isr` — Trao semaphore từ ISR | Target |
| [`10-02-mutex-priority-inheritance`](../../examples/10-02-mutex-priority-inheritance/README.md) | `10-02-mutex-priority-inheritance` — Mutex và kế thừa ưu tiên | Target |
| [`11-task-suspend-resume`](../../examples/11-task-suspend-resume/README.md) | `11-task-suspend-resume` — Tạm dừng và tiếp tục tác vụ | Target |
| [`12-software-timer`](../../examples/12-software-timer/README.md) | `12-software-timer` — Dịch vụ bộ định thời phần mềm | Target |
| [`13-01-event-post`](../../examples/13-01-event-post/README.md) | `13-01-event-post` — Đăng sự kiện haievent từ ISR | Target |
| [`13-02-active-object`](../../examples/13-02-active-object/README.md) | `13-02-active-object` — Active Object Ping–Pong | Target |
| [`13-03-flat-state-machine`](../../examples/13-03-flat-state-machine/README.md) | `13-03-flat-state-machine` — Máy trạng thái phẳng | Target |
| [`13-04-time-event`](../../examples/13-04-time-event/README.md) | `13-04-time-event` — Sự kiện thời gian haievent | Target |
| [`13-05-publish-subscribe`](../../examples/13-05-publish-subscribe/README.md) | `13-05-publish-subscribe` — Publish–Subscribe và quyền sở hữu sự kiện động | Target |
| [`13-06-event-driven-demo`](../../examples/13-06-event-driven-demo/README.md) | `13-06-event-driven-demo` — Demo haievent tích hợp | Target |
| [`14-memory-allocator-lab`](../../examples/14-memory-allocator-lab/README.md) | `14-memory-allocator-lab` — Bài thực hành bộ cấp phát bộ nhớ | Host + target |
| [`15-kernel-benchmark`](../../examples/15-kernel-benchmark/README.md) | `15-kernel-benchmark` — Benchmark kernel | Target |
| [`16-diagnostics-stress-stabilization`](../../examples/16-diagnostics-stress-stabilization/README.md) | `16-diagnostics-stress-stabilization` — Chẩn đoán và ổn định bằng stress test | Host + target |

## Dependency progression

Examples 03–06 xây stack/start/context/scheduler trước khi 07 thêm kernel tick blocking. 09–12 thêm synchronization từng primitive. 13-* chỉ xuất hiện sau khi queue/timer/context đủ để Active Object chạy đúng. 14 allocator tách khỏi kernel; 15 benchmark đo; 16 stress/diagnostics tích hợp.

## CMake truth

Environment, module set và feature define của từng example được định nghĩa trong `cmake/hairtos_examples.cmake`, là source-of-truth cho build composition.

## Validation

Host validation baseline: examples 02/14/16 PASS. Target examples cần ARM toolchain và Blue Pill.
