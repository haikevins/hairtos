# `10-01-semaphore-from-isr` — Trao semaphore từ ISR

> **Môi trường:** Target  
> **Source:** `examples/10-01-semaphore-from-isr/main.c`  
> **Trọng tâm:** ISR-to-task synchronization

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

Hardware/bare-metal tick ISR give semaphore; task block chờ và `higher_priority_task_woken` quyết định PendSV.

Example này không được hiểu như một application production. Nó cố ý cô lập một cơ chế để người học nhìn thấy **state transition và scheduling consequence** mà không bị che bởi middleware lớn. Những log/PASS check trong `main.c` là executable documentation: nếu invariant bị vi phạm, example gọi `board_panic()` hoặc trả failure trên host.

<a id="build-graph"></a>
## Build graph và cấu hình

- Environment được CMake khai báo: **Target**.
- Module được link cho example này: `platform`, `task_kernel`, `kernel_runtime`, `kernel_time`, `semaphore`.
- Target tham chiếu: `bluepill_f103c8` — STM32F103C8T6 / Cortex-M3 / 72 MHz nominal / USART1 115200 / LED PC13 active-low.

### Compile-time / source constants

| Symbol | Giá trị trong `main.c` |
| --- | --- |
| `WAITER_TASK_PRIORITY` | `1U` |
| `TRIGGER_TASK_PRIORITY` | `3U` |
| `TASK_STACK_WORDS` | `192U` |
| `TRIGGER_PERIOD_TICKS` | `500U` |
| `STM32F1_EXTI_BASE` | `0x40010400UL` |
| `STM32F1_NVIC_ISER0` | `0xE000E100UL` |
| `STM32F1_EXTI_IMR` | `STM32F1_REG32(STM32F1_EXTI_BASE + 0x00UL)` |
| `STM32F1_EXTI_SWIER` | `STM32F1_REG32(STM32F1_EXTI_BASE + 0x10UL)` |
| `STM32F1_EXTI_PR` | `STM32F1_REG32(STM32F1_EXTI_BASE + 0x14UL)` |
| `STM32F1_NVIC_ISER0_REG` | `STM32F1_REG32(STM32F1_NVIC_ISER0)` |
| `STM32F1_EXTI_LINE0` | `(1UL << 0U)` |
| `STM32F1_EXTI0_IRQ_BIT` | `(1UL << 6U)` |

### CMake feature overrides

- Example dùng default config trừ những module/definition được khai báo trong `cmake/hairtos_examples.cmake`.

<a id="runtime"></a>
## Luồng thực thi

```mermaid
stateDiagram-v2
    [*] --> AVAILABLE: count > 0
    AVAILABLE --> AVAILABLE: take / count--
    AVAILABLE --> EMPTY: last token taken
    EMPTY --> WAITING: task take with timeout
    WAITING --> EMPTY: more waiters remain
    WAITING --> AVAILABLE: give with no remaining waiter
    EMPTY --> AVAILABLE: give increments count
```

Để hiểu runtime thật, đọc sơ đồ cùng `main.c` và module source. Các điểm chuyển task state/context không diễn ra trong application code đơn lẻ mà qua kernel + architecture port.

### Các chi tiết quan sát trực tiếp từ example

- Sử dụng API semaphore không blocking trong ISR.
- Truyền `higher_priority_task_woken` ra khỏi kernel API.
- Pend context switch sau ISR bằng `hr_yield_from_isr()`.
- Phân biệt ISR context với task context.
- Binary semaphore là counting semaphore max count 1.
- ISR không được block hoặc dùng finite timeout.
- Context switch không xảy ra giữa ISR handler; PendSV chạy sau exception return.
- EXTI0 được trigger bằng SWIER nên không cần nút ngoài.
- `hairtos/hr_semaphore.h`
- `hairtos/hr_context.h`
- `stm32f1.h`
- `hr_semaphore_create_binary()`
- `hr_semaphore_take()`
- `hr_semaphore_give_from_isr()`
- `hr_yield_from_isr()`
- `task_kernel`
- `kernel_runtime`
- `kernel_time`
- Phần cứng — STM32F103C8T6 Blue Pill — Chạy firmware target.
- Nạp/debug — ST-Link V2 qua SWD — Dùng OpenOCD để flash, verify và reset.
- UART — USART1, PA9 TX / PA10 RX, 115200 8-N-1 — Theo dõi log và trạng thái PASS/FAIL.
- LED — PC13, active-low — Hiển thị heartbeat hoặc trạng thái quan sát.
- `waiter` — Priority 1, stack 192 words — Block trên binary semaphore.
- `trigger` — Priority 3, stack 192 words — Mỗi 500 ticks ghi EXTI_SWIER.

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
- `hr_port_yield_from_isr()`
- `hr_semaphore_create_binary()`
- `hr_semaphore_give_from_isr()`
- `hr_semaphore_take()`
- `hr_task_create_static()`
- `hr_task_current()`
- `hr_task_delay_until()`
- `hr_task_start()`
- `hr_time_now()`

Ownership cần nhớ:

- `hr_task_t`, stack, queue/semaphore/mutex/timer object và haievent storage trong examples đều là static/caller-owned.
- API kernel giữ pointer tới storage này sau create, vì vậy lifetime phải kéo dài toàn bộ thời gian object còn active.
- ISR path không được gọi blocking API. API `_from_isr` chỉ làm bounded work và trả `higher_priority_task_woken` để PendSV xử lý switch sau ISR.
- Dynamic haievent event từ pool dùng retain/release; static event không được framework tự free.

<a id="pass"></a>
## Invariant và PASS criteria

- `take` tiêu thụ token nếu count > 0; nếu không có token thì có thể block theo timeout.
- `give` với waiter đang chờ chuyển quyền tiến triển trực tiếp sang waiter; nếu không có waiter mới tăng count.
- `give_from_isr` là ISR-safe và chỉ báo nhu cầu context switch qua output flag.
- Semaphore không theo dõi owner và không có priority inheritance; dùng mutex nếu cần mutual exclusion có ownership.
- Give khi count đã max và không có waiter trả `HR_ERROR_SEMAPHORE_FULL`.

Các check/log cứng trong source:

- `ERROR: semaphore ISR handoff failed.`
- `ERROR: trigger delay failed.`
- `Semaphore creation failed.`
- `Kernel initialization failed.`
- `Semaphore task setup failed.`
- `ERROR: hr_kernel_start returned status=`

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
make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr build
make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr run
make TARGET=bluepill_f103c8 EXAMPLE=10-01-semaphore-from-isr check
```

<a id="source-map"></a>
## Source map và references

- `examples/10-01-semaphore-from-isr/main.c`
- `cmake/hairtos_examples.cmake`
- `kernel/src/hr_semaphore.c`
- `kernel/internal/hr_semaphore_internal.h`
- `tests/host/test_semaphore.c`

### Tài liệu tham khảo

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Nguồn implementation trong repository:**
- `kernel/src/hr_semaphore.c`
- `kernel/internal/hr_semaphore_internal.h`
- `tests/host/test_semaphore.c`
