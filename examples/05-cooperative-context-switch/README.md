# `05-cooperative-context-switch` — Chuyển ngữ cảnh hợp tác

> **Môi trường:** Target  
> **Source:** `examples/05-cooperative-context-switch/main.c`  
> **Trọng tâm:** PendSV context switch hợp tác

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

Hai task cùng priority tự gọi yield; stack-local counter/cookie chứng minh context R4-R11 + hardware frame được bảo toàn.

Example này không được hiểu như một application production. Nó cố ý cô lập một cơ chế để người học nhìn thấy **state transition và scheduling consequence** mà không bị che bởi middleware lớn. Những log/PASS check trong `main.c` là executable documentation: nếu invariant bị vi phạm, example gọi `board_panic()` hoặc trả failure trên host.

<a id="build-graph"></a>
## Build graph và cấu hình

- Environment được CMake khai báo: **Target**.
- Module được link cho example này: `platform`, `baremetal_tick`, `task_kernel`, `kernel_runtime`.
- Target tham chiếu: `bluepill_f103c8` — STM32F103C8T6 / Cortex-M3 / 72 MHz nominal / USART1 115200 / LED PC13 active-low.

### Compile-time / source constants

| Symbol | Giá trị trong `main.c` |
| --- | --- |
| `COOPERATIVE_TASK_PRIORITY` | `2U` |
| `TASK_STACK_WORDS` | `160U` |
| `TASK_PRINT_DELAY_MS` | `250U` |

### CMake feature overrides

- Example dùng default config trừ những module/definition được khai báo trong `cmake/hairtos_examples.cmake`.

<a id="runtime"></a>
## Luồng thực thi

```mermaid
sequenceDiagram
    participant T as Current task / PSP
    participant CPU as Cortex-M3 hardware
    participant P as PendSV_Handler
    participant K as hr_kernel_select_next_from_pendsv()
    T->>CPU: PendSV pending
    CPU->>CPU: stack R0-R3,R12,LR,PC,xPSR on PSP
    CPU->>P: enter Handler mode on MSP
    P->>P: save R4-R11 to current PSP
    P->>K: select next TCB
    K-->>P: g_hr_current_task_control_block updated
    P->>P: restore R4-R11 from next PSP
    P->>CPU: exception return 0xFFFFFFFD
    CPU->>T: unstack hardware frame and resume next task
```

Để hiểu runtime thật, đọc sơ đồ cùng `main.c` và module source. Các điểm chuyển task state/context không diễn ra trong application code đơn lẻ mà qua kernel + architecture port.

### Các chi tiết quan sát trực tiếp từ example

- Save PSP và R4–R11 của task đang chạy.
- Restore context của task kế tiếp.
- Xác nhận local variable và stack cookie của mỗi task được bảo toàn.
- Hiểu cooperative scheduling: task chỉ chuyển khi tự yield.
- Hardware tự stack R0–R3, R12, LR, PC, xPSR.
- Port assembly stack/unstack R4–R11.
- TCB lưu saved PSP.
- Equal-priority FIFO được rotate khi yield.
- `hairtos/hr_kernel.h`
- `hairtos/hr_task.h`
- `hr_port.h`
- `hr_task_yield()`
- `hr_task_current()`
- `hr_kernel_start()`
- `task_kernel`
- `kernel_runtime`
- `baremetal_tick`
- Dòng A/B xen kẽ ổn định.
- Phần cứng — STM32F103C8T6 Blue Pill — Chạy firmware target.
- Nạp/debug — ST-Link V2 qua SWD — Dùng OpenOCD để flash, verify và reset.
- UART — USART1, PA9 TX / PA10 RX, 115200 8-N-1 — Theo dõi log và trạng thái PASS/FAIL.
- LED — PC13, active-low — Hiển thị heartbeat hoặc trạng thái quan sát.
- `task-a` — Priority 2, stack 160 words — Counter bắt đầu từ 0 và tăng 1.
- `task-b` — Priority 2, stack 160 words — Counter bắt đầu từ 1000 và tăng 10.

<a id="api"></a>
## API và ownership

API được gọi trực tiếp trong `main.c` (đã trích từ source):

- `board_delay_ms()`
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
- `hr_task_start()`
- `hr_task_yield()`

Ownership cần nhớ:

- `hr_task_t`, stack, queue/semaphore/mutex/timer object và haievent storage trong examples đều là static/caller-owned.
- API kernel giữ pointer tới storage này sau create, vì vậy lifetime phải kéo dài toàn bộ thời gian object còn active.
- ISR path không được gọi blocking API. API `_from_isr` chỉ làm bounded work và trả `higher_priority_task_woken` để PendSV xử lý switch sau ISR.
- Dynamic haievent event từ pool dùng retain/release; static event không được framework tự free.

<a id="pass"></a>
## Invariant và PASS criteria

- TCB đặt `stack_pointer` ở offset 0 và có `_Static_assert` để assembly có thể load/store saved PSP mà không cần biết layout C còn lại.
- Initial stack frame được dựng giống exception-return frame thật; top stack được align xuống 8 byte.
- Thread mode sau SVC chạy privileged với PSP (`CONTROL.SPSEL=1`); handler mode tiếp tục dùng MSP.
- PendSV được cấu hình priority thấp nhất để việc chọn next task không cắt ngang exception quan trọng hơn.
- Port hiện không lưu FPU context vì `HR_CFG_USE_FPU=0` và target Cortex-M3 không có FPU.

Các check/log cứng trong source:

- `ERROR: wrong current task in `
- `ERROR: cooperative task is not using PSP.`
- `ERROR: task A stack-local state was corrupted.`
- `ERROR: task B stack-local state was corrupted.`
- `Kernel initialization failed.`
- `Task A creation failed.`
- `Task B creation failed.`
- `Task registration failed.`
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
make TARGET=bluepill_f103c8 EXAMPLE=05-cooperative-context-switch build
make TARGET=bluepill_f103c8 EXAMPLE=05-cooperative-context-switch run
make TARGET=bluepill_f103c8 EXAMPLE=05-cooperative-context-switch check
```

<a id="source-map"></a>
## Source map và references

- `examples/05-cooperative-context-switch/main.c`
- `cmake/hairtos_examples.cmake`
- `arch/arm/cortex-m3/hr_portasm.S`
- `arch/arm/cortex-m3/hr_port_stack.c`
- `arch/arm/cortex-m3/hr_port.c`
- `kernel/internal/hr_task_internal.h`
- `tests/host/test_port_stack.c`

### Tài liệu tham khảo

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)

**Nguồn implementation trong repository:**
- `arch/arm/cortex-m3/hr_portasm.S`
- `arch/arm/cortex-m3/hr_port_stack.c`
- `arch/arm/cortex-m3/hr_port.c`
- `kernel/internal/hr_task_internal.h`
- `tests/host/test_port_stack.c`
