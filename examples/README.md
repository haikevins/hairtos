# Examples của hairtos

Thư mục `examples/` chứa các bài thực hành theo thứ tự phát triển từ nền tảng bare-metal đến diagnostics/stress tích hợp. Mỗi thư mục có một `main.c` độc lập và một README dùng chung bố cục 10 mục:

1. Mục tiêu học tập.
2. Kiến thức trọng tâm.
3. Thành phần và cấu hình.
4. Luồng thực thi.
5. API và mã nguồn liên quan.
6. Build, run và kiểm tra.
7. Kết quả mong đợi.
8. Tiêu chí PASS và xử lý lỗi.
9. Giới hạn của example.
10. Liên hệ với lộ trình.

## Quy ước môi trường

- **Host:** chạy executable native trên máy phát triển; không flash STM32.
- **Target:** cross-build và chạy trên STM32F103C8T6 Blue Pill.
- **Host + Target:** phải chọn `ENVIRONMENT=host` hoặc `ENVIRONMENT=target` khi muốn rõ biến thể.
- `run` trên host chạy binary; `run` trên target build, flash, verify và reset qua OpenOCD.

## Lệnh chung

```bash
make help
make list-examples
make EXAMPLE=<name> build
make EXAMPLE=<name> run
make EXAMPLE=<name> check
make EXAMPLE=<name> clean
make host-tests
make clean-all
```

Với example hỗ trợ hai môi trường:

```bash
make ENVIRONMENT=host   EXAMPLE=14-memory-allocator-lab run
make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run
```

## Phần cứng chung cho target

| Thành phần | Kết nối / cấu hình |
|---|---|
| Board | STM32F103C8T6 Blue Pill |
| Debugger | ST-Link V2: SWDIO, SWCLK, GND, 3.3 V reference |
| UART log | USART1 PA9 TX → USB-UART RX, PA10 RX ← USB-UART TX, GND chung |
| Terminal | 115200 baud, 8 data bits, no parity, 1 stop bit |
| LED | PC13, active-low |

## Danh sách example

| Example | Môi trường | Nội dung | Lệnh chạy chính |
| --- | --- | --- | --- |
| [`01-baremetal-foundation`](01-baremetal-foundation/README.md) | Target | Bare-metal Foundation | `make EXAMPLE=01-baremetal-foundation run` |
| [`02-kernel-data-structures-host`](02-kernel-data-structures-host/README.md) | Host | Kernel Data Structures — Host Demo | `make EXAMPLE=02-kernel-data-structures-host run` |
| [`03-static-task-stack`](03-static-task-stack/README.md) | Target | Static TCB and Initial Task Stack | `make EXAMPLE=03-static-task-stack run` |
| [`04-start-first-task`](04-start-first-task/README.md) | Target | Start the First Task with SVC | `make EXAMPLE=04-start-first-task run` |
| [`05-cooperative-context-switch`](05-cooperative-context-switch/README.md) | Target | Cooperative Context Switch | `make EXAMPLE=05-cooperative-context-switch run` |
| [`06-priority-scheduler`](06-priority-scheduler/README.md) | Target | Fixed-Priority Scheduler | `make EXAMPLE=06-priority-scheduler run` |
| [`07-task-delay-timeout`](07-task-delay-timeout/README.md) | Target | SysTick, Task Delay and Timeout | `make EXAMPLE=07-task-delay-timeout run` |
| [`08-preemption-round-robin`](08-preemption-round-robin/README.md) | Target | Preemption and Round-Robin | `make EXAMPLE=08-preemption-round-robin run` |
| [`09-queue-blocking-ipc`](09-queue-blocking-ipc/README.md) | Target | Queue and Blocking IPC | `make EXAMPLE=09-queue-blocking-ipc run` |
| [`10-01-semaphore-from-isr`](10-01-semaphore-from-isr/README.md) | Target | Semaphore Give from ISR | `make EXAMPLE=10-01-semaphore-from-isr run` |
| [`10-02-mutex-priority-inheritance`](10-02-mutex-priority-inheritance/README.md) | Target | Mutex and Priority Inheritance | `make EXAMPLE=10-02-mutex-priority-inheritance run` |
| [`11-task-suspend-resume`](11-task-suspend-resume/README.md) | Target | Task Suspend and Resume | `make EXAMPLE=11-task-suspend-resume run` |
| [`12-software-timer`](12-software-timer/README.md) | Target | Software Timer Service | `make EXAMPLE=12-software-timer run` |
| [`13-01-event-post`](13-01-event-post/README.md) | Target | haievent Post from ISR | `make EXAMPLE=13-01-event-post run` |
| [`13-02-active-object`](13-02-active-object/README.md) | Target | Active Object Ping–Pong | `make EXAMPLE=13-02-active-object run` |
| [`13-03-flat-state-machine`](13-03-flat-state-machine/README.md) | Target | Flat State Machine | `make EXAMPLE=13-03-flat-state-machine run` |
| [`13-04-time-event`](13-04-time-event/README.md) | Target | haievent Time Event | `make EXAMPLE=13-04-time-event run` |
| [`13-05-publish-subscribe`](13-05-publish-subscribe/README.md) | Target | Publish–Subscribe and Dynamic Event Ownership | `make EXAMPLE=13-05-publish-subscribe run` |
| [`13-06-event-driven-demo`](13-06-event-driven-demo/README.md) | Target | Integrated haievent Demo | `make EXAMPLE=13-06-event-driven-demo run` |
| [`14-memory-allocator-lab`](14-memory-allocator-lab/README.md) | Host + Target | Memory Allocator Lab | `make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run` |
| [`15-kernel-benchmark`](15-kernel-benchmark/README.md) | Target | Kernel Benchmark | `make EXAMPLE=15-kernel-benchmark run` |
| [`16-diagnostics-stress-stabilization`](16-diagnostics-stress-stabilization/README.md) | Host + Target | Diagnostics and Stress Stabilization | `make ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization run` |

## Cách đọc kết quả

- Một target **build PASS** chưa chứng minh runtime trên board PASS.
- Dòng `PASS` trong UART là checkpoint do chính example kiểm tra các invariant chính.
- Giá trị tick/counter có thể khác đôi chút; thứ tự sự kiện và quan hệ priority mới là tiêu chí quan trọng.
- Khi có `board_panic()`, giữ lại UART log, map file và fault record trước khi thay đổi code.
- Host tests với ASan/UBSan bổ sung kiểm tra memory/invariant nhưng không thay thế test timing trên Cortex-M3.
