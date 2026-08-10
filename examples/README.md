# Các ví dụ của hairtos

## 1. Mục đích

`examples/` là lộ trình thực hành từ nền tảng bare-metal đến kernel tích hợp, benchmark và diagnostics. Mỗi thư mục có `main.c` và README theo cùng bố cục mười phần.

## 2. Quy ước môi trường

- **Host-only:** chạy bằng compiler native và sanitizer; không cần board.
- **Target-only:** cross-build cho target manifest được chọn và chạy trên MCU.
- **Dual:** cùng một chủ đề có biến thể host và target.

CMake tự suy luận môi trường khi example chỉ hỗ trợ một loại. Với dual example, nên ghi rõ `ENVIRONMENT=host` hoặc `ENVIRONMENT=target`.

## 3. Quy ước target

```bash
make list-targets
make TARGET=<target> EXAMPLE=<name> build
```

Target mặc định là `bluepill_f103c8`. Các chi tiết như clock, UART, LED, marker, startup, linker script và OpenOCD được lấy từ target manifest/board implementation; chúng không phải contract chung cho mọi MCU.

## 4. Lệnh chung

```bash
make TARGET=bluepill_f103c8 EXAMPLE=<name> build
make TARGET=bluepill_f103c8 EXAMPLE=<name> run
make TARGET=bluepill_f103c8 EXAMPLE=<name> check
make TARGET=bluepill_f103c8 EXAMPLE=<name> clean
make TARGET=bluepill_f103c8 EXAMPLE=<name> intellisense
```

Dual example:

```bash
make TARGET=bluepill_f103c8 ENVIRONMENT=host   EXAMPLE=<name> run
make TARGET=bluepill_f103c8 ENVIRONMENT=target EXAMPLE=<name> run
```

## 5. Danh sách ví dụ

| Example | Môi trường | Nội dung chính |
| --- | --- | --- |
| [`01-baremetal-foundation`](01-baremetal-foundation/README.md) | Target | Board clock, GPIO, UART và bare-metal tick |
| [`02-kernel-data-structures-host`](02-kernel-data-structures-host/README.md) | Host | Intrusive list, ready set và wait list |
| [`03-static-task-stack`](03-static-task-stack/README.md) | Target | TCB tĩnh và initial stack frame |
| [`04-start-first-task`](04-start-first-task/README.md) | Target | Khởi chạy task đầu tiên qua port |
| [`05-cooperative-context-switch`](05-cooperative-context-switch/README.md) | Target | Yield và context switch hợp tác |
| [`06-priority-scheduler`](06-priority-scheduler/README.md) | Target | Fixed-priority scheduler |
| [`07-task-delay-timeout`](07-task-delay-timeout/README.md) | Target | Tick, delay và timeout |
| [`08-preemption-round-robin`](08-preemption-round-robin/README.md) | Target | Preemption và time slicing |
| [`09-queue-blocking-ipc`](09-queue-blocking-ipc/README.md) | Target | Queue và blocking IPC |
| [`10-01-semaphore-from-isr`](10-01-semaphore-from-isr/README.md) | Target | Semaphore wakeup từ ISR |
| [`10-02-mutex-priority-inheritance`](10-02-mutex-priority-inheritance/README.md) | Target | Mutex và priority inheritance |
| [`11-task-suspend-resume`](11-task-suspend-resume/README.md) | Target | Suspend/resume task |
| [`12-software-timer`](12-software-timer/README.md) | Target | Software timer service |
| [`13-01-event-post`](13-01-event-post/README.md) | Target | Event posting |
| [`13-02-active-object`](13-02-active-object/README.md) | Target | Active Object |
| [`13-03-flat-state-machine`](13-03-flat-state-machine/README.md) | Target | Flat state machine |
| [`13-04-time-event`](13-04-time-event/README.md) | Target | Time event |
| [`13-05-publish-subscribe`](13-05-publish-subscribe/README.md) | Target | Publish/subscribe |
| [`13-06-event-driven-demo`](13-06-event-driven-demo/README.md) | Target | Demo `haievent` tích hợp |
| [`14-memory-allocator-lab`](14-memory-allocator-lab/README.md) | Host + Target | Fixed pool và first-fit heap |
| [`15-kernel-benchmark`](15-kernel-benchmark/README.md) | Target | Benchmark backend và footprint |
| [`16-diagnostics-stress-stabilization`](16-diagnostics-stress-stabilization/README.md) | Host + Target | Diagnostics, fault và scheduler stress |

## 6. Trình tự dùng khi port target mới

1. Example 01: clock, board, UART, GPIO và tick bare-metal.
2. Example 03: initial stack frame.
3. Example 04–06: first task và context switch.
4. Example 07–08: tick, preemption và round-robin.
5. Example 09–12: IPC và timer.
6. Example 13: framework event-driven.
7. Example 15: benchmark clock/marker/footprint.
8. Example 16: diagnostics, fault retention và soak workload.

## 7. Cách đọc kết quả

- Output trong README là mẫu, không phải giá trị timing cố định.
- Pin/clock/CPU name thay đổi theo target.
- Build PASS chỉ xác nhận compile/link.
- Runtime PASS cần UART/LED/debugger hoặc measurement trên board.
- Host sanitizer PASS không chứng minh exception/context backend của target.

## Liên hệ Version 2

Examples v1 tiếp tục đóng vai trò regression ladder cho Version 2. Target mới phải đi từ bare-metal đến context/tick/IPC/framework thay vì chỉ build image tích hợp.

Roadmap v2: [`../docs/09-version2/roadmap.md`](../docs/09-version2/roadmap.md).
