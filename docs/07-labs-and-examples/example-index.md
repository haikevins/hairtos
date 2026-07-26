# Danh mục example

## 1. Mục tiêu

Tài liệu này phân loại example theo môi trường chạy và chuẩn hóa toàn bộ lệnh Make.

## 2. Bố cục lệnh thống nhất

```bash
make EXAMPLE=<name> build
make EXAMPLE=<name> run
make EXAMPLE=<name> check
make EXAMPLE=<name> clean
```

Ý nghĩa của `run` phụ thuộc môi trường:

- Host: chạy executable native.
- Target: build firmware, flash bằng OpenOCD, verify và reset STM32.

`ENVIRONMENT=auto` là mặc định. Phase 2 tự chọn host; các example còn lại mặc định chọn target. Với Phase 14 và 16, dùng `ENVIRONMENT=host` để chọn bản native.

## 3. Danh mục

| Example | Môi trường | Mục tiêu | Lệnh chạy |
|---|---|---|---|
| `01-baremetal-foundation` | Target | Bare-metal clock, GPIO, UART, and temporary SysTick | `make EXAMPLE=01-baremetal-foundation run` |
| `02-kernel-data-structures-host` | Host | Intrusive lists, ready set, and wait-list ordering | `make EXAMPLE=02-kernel-data-structures-host run` |
| `03-static-task-stack` | Target | TCB and Cortex-M3 initial task stack | `make EXAMPLE=03-static-task-stack run` |
| `04-start-first-task` | Target | Start the first task through SVC | `make EXAMPLE=04-start-first-task run` |
| `05-cooperative-context-switch` | Target | PendSV cooperative switching | `make EXAMPLE=05-cooperative-context-switch run` |
| `06-priority-scheduler` | Target | Fixed-priority scheduler and equal-priority FIFO | `make EXAMPLE=06-priority-scheduler run` |
| `07-task-delay-timeout` | Target | Kernel SysTick, delay, and timeout wake-up | `make EXAMPLE=07-task-delay-timeout run` |
| `08-preemption-round-robin` | Target | Preemption and round-robin | `make EXAMPLE=08-preemption-round-robin run` |
| `09-queue-blocking-ipc` | Target | Blocking queue and timeout | `make EXAMPLE=09-queue-blocking-ipc run` |
| `10-01-semaphore-from-isr` | Target | Semaphore give from ISR | `make EXAMPLE=10-01-semaphore-from-isr run` |
| `10-02-mutex-priority-inheritance` | Target | Mutex ownership and priority inheritance | `make EXAMPLE=10-02-mutex-priority-inheritance run` |
| `11-task-suspend-resume` | Target | Suspend and resume | `make EXAMPLE=11-task-suspend-resume run` |
| `12-software-timer` | Target | Software timer service | `make EXAMPLE=12-software-timer run` |
| `13-01-event-post` | Target | Post haievent from ISR | `make EXAMPLE=13-01-event-post run` |
| `13-02-active-object` | Target | Active Object dispatch | `make EXAMPLE=13-02-active-object run` |
| `13-03-flat-state-machine` | Target | Flat state machine | `make EXAMPLE=13-03-flat-state-machine run` |
| `13-04-time-event` | Target | haievent time event | `make EXAMPLE=13-04-time-event run` |
| `13-05-publish-subscribe` | Target | Publish/subscribe ownership | `make EXAMPLE=13-05-publish-subscribe run` |
| `13-06-event-driven-demo` | Target | Integrated haievent demo | `make EXAMPLE=13-06-event-driven-demo run` |
| `14-memory-allocator-lab` | Host + Target | Fixed-block pool and first-fit heap lab | Host: `make ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run`<br>Target: `make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run` |
| `15-kernel-benchmark` | Target | DWT/GPIO kernel benchmark | `make EXAMPLE=15-kernel-benchmark run` |
| `16-diagnostics-stress-stabilization` | Host + Target | Diagnostics and deterministic stress | Host: `make ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run`<br>Target: `make ENVIRONMENT=target EXAMPLE=16-diagnostics-stress-stabilization run` |

## 4. Các lệnh chung

```bash
make help
make list-examples
make host-tests
make clean-all
```

## 5. Ví dụ sử dụng

```bash
# Host-only
make EXAMPLE=02-kernel-data-structures-host build
make EXAMPLE=02-kernel-data-structures-host run

# STM32 target
make EXAMPLE=09-queue-blocking-ipc build
make EXAMPLE=09-queue-blocking-ipc run

# Một example có cả host và target
make ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run
make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run
```

## 6. Validation

```bash
make EXAMPLE=02-kernel-data-structures-host check
make TOOLCHAIN=clang EXAMPLE=16-diagnostics-stress-stabilization check
```

`check` chạy host tests bằng ASan/UBSan rồi build example đã chọn. Target cross-build thành công chỉ chứng minh compile/link; runtime, interrupt và timing vẫn phải xác nhận trên board.
