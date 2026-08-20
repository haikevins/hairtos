# `12-software-timer` — Dịch vụ bộ định thời phần mềm

> **Môi trường:** Target  
> **Source:** `examples/12-software-timer/main.c`  
> **Trọng tâm:** Timer-service task

[← Root README](../../README.md)

## Mục lục

- [Mục tiêu và bản chất](#muc-tieu)
- [Build graph và cấu hình](#build-graph)
- [Luồng thực thi](#runtime)
- [API và ownership](#api)
- [Invariant / PASS criteria](#pass)
- [Debug và failure modes](#debug)
- [Validation](#validation)
- [Source map và references](#source-map)

<a id="muc-tieu"></a>
## Mục tiêu và bản chất

Expiry xử lý từ tick ISR nhưng callback chạy trong timer task; demo one-shot/periodic/reset/change/stop semantics.

Example này không được hiểu như một application production. Nó cố ý cô lập một cơ chế để người học nhìn thấy **state transition và scheduling consequence** mà không bị che bởi middleware lớn. Những log/PASS check trong `main.c` là executable documentation: nếu invariant bị vi phạm, example gọi `board_panic()` hoặc trả failure trên host.

<a id="build-graph"></a>
## Build graph và cấu hình

- Environment được CMake khai báo: **Target**.
- Module được link cho example này: `platform`, `task_kernel`, `kernel_runtime`, `kernel_time`, `semaphore`, `timer`.
- Target tham chiếu: `bluepill_f103c8` — STM32F103C8T6 / Cortex-M3 / 72 MHz nominal / USART1 115200 / LED PC13 active-low.

### Compile-time / source constants

| Symbol | Giá trị trong `main.c` |
| --- | --- |
| `CONTROL_TASK_PRIORITY` | `3U` |
| `CONTROL_TASK_STACK_WORDS` | `224U` |

### CMake feature overrides

- Software timer được bật cho build này; timer-service task priority được override thành 1.

<a id="runtime"></a>
## Luồng thực thi

```mermaid
sequenceDiagram
    participant ST as SysTick ISR
    participant TL as Timer timeout list
    participant TS as Timer-service task
    participant CB as User callback
    ST->>TL: advance(now)
    TL-->>ST: expired timer(s)
    ST->>ST: pending_count++ / queue pending node
    ST->>TS: wake service task if needed
    TS->>TS: pop one pending timer
    TS->>CB: callback(argument)
    CB-->>TS: return in task context
```

Để hiểu runtime thật, đọc sơ đồ cùng `main.c` và module source. Các điểm chuyển task state/context không diễn ra trong application code đơn lẻ mà qua kernel + architecture port.

### Các chi tiết quan sát trực tiếp từ example

- Tạo timer tĩnh.
- Start, reset, change period và stop timer.
- Phân biệt timer expiration trong SysTick với callback execution trong task.
- Kiểm tra one-shot chỉ callback một lần và periodic tự rearm.
- Danh sách deadline của timer được sắp thứ tự.
- Pending callback list và timer-service semaphore.
- Callback không chạy trong ISR.
- Periodic rearm từ deadline để hạn chế drift.
- `hairtos/hr_timer.h`
- `hairtos/hr_time.h`
- `hr_timer_create_static()`
- `hr_timer_start()`
- `hr_timer_reset()`
- `hr_timer_change_period()`
- `hr_timer_stop()`
- `task_kernel`
- `kernel_runtime`
- `kernel_time`
- Phần cứng — STM32F103C8T6 Blue Pill — Chạy firmware target.
- Nạp/debug — ST-Link V2 qua SWD — Dùng OpenOCD để flash, verify và reset.
- UART — USART1, PA9 TX / PA10 RX, 115200 8-N-1 — Theo dõi log và trạng thái PASS/FAIL.
- LED — PC13, active-low — Hiển thị heartbeat hoặc trạng thái quan sát.
- `timer-control` — Priority 3, stack 224 words — Điều khiển start/reset/stop.
- `periodic` — 250 ticks → 500 ticks, auto reload — Toggle LED và đếm callback.

<a id="api"></a>
## API và ownership

API được gọi trực tiếp trong `main.c` (đã trích từ source):

- `board_init()`
- `board_led_toggle()`
- `board_panic()`
- `board_uart_write_line()`
- `board_uart_write_string()`
- `board_uart_write_u32()`
- `hr_kernel_init()`
- `hr_kernel_start()`
- `hr_port_thread_uses_psp()`
- `hr_task_create_static()`
- `hr_task_current()`
- `hr_task_delay()`
- `hr_task_start()`
- `hr_time_now()`
- `hr_timer_change_period()`
- `hr_timer_create_static()`
- `hr_timer_reset()`
- `hr_timer_start()`
- `hr_timer_stop()`

Ownership cần nhớ:

- `hr_task_t`, stack, queue/semaphore/mutex/timer object và haievent storage trong examples đều là static/caller-owned.
- API kernel giữ pointer tới storage này sau create, vì vậy lifetime phải kéo dài toàn bộ thời gian object còn active.
- ISR path không được gọi blocking API. API `_from_isr` chỉ làm bounded work và trả `higher_priority_task_woken` để PendSV xử lý switch sau ISR.
- Dynamic haievent event từ pool dùng retain/release; static event không được framework tự free.

<a id="pass"></a>
## Invariant và PASS criteria

- Timer object là static opaque storage; một timer có name, period, auto_reload, callback, argument và timeout node.
- Timer-service task được tạo lazily khi timer subsystem cần initialize và dùng priority/stack từ config.
- Expiry ISR path chỉ cập nhật state/pending và wake service task; callback không được chạy trong handler mode.
- One-shot trở inactive sau expiry; periodic timer được re-arm theo period.
- `stop/reset/change_period` phải xử lý cả active timeout node và pending callbacks một cách có chủ đích.

Các check/log cứng trong source:

- `ERROR: software timer callback count mismatch.`
- `Software timer service: PASS`
- `Software timer setup failed.`

<a id="debug"></a>
## Debug và failure modes

- Nếu target treo trong `board_panic()`, xem UART log ngay trước đó rồi attach GDB/OpenOCD để kiểm tra current task, PSP/MSP, ready bitmap và fault record nếu diagnostics bật.
- Nếu behavior sai chỉ khi optimize/timing thay đổi, kiểm tra race giữa task/ISR, critical-section scope và việc log UART làm nhiễu thời gian.
- Nếu task không chạy, phân biệt CREATED/READY/BLOCKED/SUSPENDED và kiểm tra task có được `hr_task_start()` hay không.
- Nếu wake không xảy ra, kiểm tra cả object wait list lẫn timeout node; một wake path không được để node stale trong structure còn lại.
- Target log là evidence runtime; build PASS chỉ là evidence compile/link.

<a id="validation"></a>
## Validation

- Example là target-only trong CMake. Môi trường audit không có `arm-none-eabi-gcc`/OpenOCD nên không tuyên bố đã build/flash lại target.
- `make TARGET=bluepill_f103c8 host-tests` đã PASS toàn bộ host suite trong audit tài liệu này.

### Lệnh chuẩn

```bash
make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer build
make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer run
make TARGET=bluepill_f103c8 EXAMPLE=12-software-timer check
```

<a id="source-map"></a>
## Source map và references

- `examples/12-software-timer/main.c`
- `cmake/hairtos_examples.cmake`
- `kernel/src/hr_timer.c`
- `kernel/internal/hr_timer_internal.h`
- `tests/host/test_timer.c`

### Tài liệu tham khảo

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Nguồn implementation trong repository:**
- `kernel/src/hr_timer.c`
- `kernel/internal/hr_timer_internal.h`
- `tests/host/test_timer.c`
