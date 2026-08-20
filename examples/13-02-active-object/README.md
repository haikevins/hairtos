# `13-02-active-object` — Active Object Ping–Pong

> **Môi trường:** Target  
> **Source:** `examples/13-02-active-object/main.c`  
> **Trọng tâm:** Active Object ping-pong

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

Hai AO độc lập, mỗi AO là task + queue + state machine; event ownership đi qua post/dispatch/release.


<a id="build-graph"></a>
## Build graph và cấu hình

- Environment được CMake khai báo: **Target**.
- Module được link cho example này: `platform`, `task_kernel`, `kernel_runtime`, `kernel_time`, `context`, `queue`, `semaphore`, `timer`, `haievent`.
- Target tham chiếu: `bluepill_f103c8` — STM32F103C8T6 / Cortex-M3 / 72 MHz nominal / USART1 115200 / LED PC13 active-low.

### Compile-time / source constants

| Symbol | Giá trị trong `main.c` |
| --- | --- |
| `STACK_WORDS` | `224U` |
| `QUEUE_LENGTH` | `4U` |

### CMake feature overrides

- Software timer được bật cho build này; timer-service task priority được override thành 1.

<a id="runtime"></a>
## Luồng thực thi

```mermaid
flowchart TB
    P["Producer / ISR / publisher"] --> POST["Post event"]
    POST --> Q["AO event queue"]
    Q --> TASK["Dedicated task"]
    TASK --> DISPATCH["RTC dispatch"]
    DISPATCH --> TRANS["Apply transition"]
    TRANS --> RELEASE["Release dynamic event"]
```


### Các chi tiết quan sát trực tiếp từ example

- Hiểu encapsulation của Active Object.
- Dùng state-machine context riêng cho mỗi actor.
- Post event giữa hai AO mà không chia sẻ control flow.
- Quan sát run-to-completion và queue-driven scheduling.
- Mỗi AO có task, stack, queue và state machine.
- Static event PING/PONG được dùng lại.
- Context chứa peer, reply event và counter.
- Starter task chỉ kick-off chuỗi event.
- `haievent/haievent.h`
- `he_active_create_static()`
- `he_active_post()`
- `he_state_machine_context()`
- `haievent`
- Hai AO đều nhận ENTRY.
- Counters của ping và pong tăng luân phiên.
- Không có queue post failure.
- Chỉ một AO chạy: kiểm tra peer pointer hoặc reply event.
- Queue đầy nhanh: UART quá chậm hoặc producer loop không bị scheduling điều tiết.
- Phần cứng — STM32F103C8T6 Blue Pill — Chạy firmware target.
- Nạp/debug — ST-Link V2 qua SWD — Dùng OpenOCD để flash, verify và reset.
- UART — USART1, PA9 TX / PA10 RX, 115200 8-N-1 — Theo dõi log và trạng thái PASS/FAIL.
- LED — PC13, active-low — Hiển thị heartbeat hoặc trạng thái quan sát.
- `ping-AO` — Priority 2, stack 224, queue 4 — Handle PING rồi post PONG.
- `pong-AO` — Priority 3, stack 224, queue 4 — Handle PONG rồi post PING.

<a id="api"></a>
## API và ownership

API được gọi trực tiếp trong `main.c` (đã trích từ source):

- `board_init()`
- `board_panic()`
- `board_uart_write_line()`
- `board_uart_write_string()`
- `board_uart_write_u32()`
- `he_active_create_static()`
- `he_active_post()`
- `he_event_init_static()`
- `he_state_machine_context()`
- `hr_kernel_init()`
- `hr_kernel_start()`
- `hr_task_create_static()`
- `hr_task_delay()`
- `hr_task_start()`

Ownership cần nhớ:

- `hr_task_t`, stack, queue/semaphore/mutex/timer object và haievent storage trong examples đều là static/caller-owned.
- API kernel giữ pointer tới storage này sau create, vì vậy lifetime phải kéo dài toàn bộ thời gian object còn active.
- ISR path không được gọi blocking API. API `_from_isr` chỉ làm bounded work và trả `higher_priority_task_woken` để PendSV xử lý switch sau ISR.
- Dynamic haievent event từ pool dùng retain/release; static event không được framework tự free.

<a id="pass"></a>
## Invariant và PASS criteria

- Một AO chạy run-to-completion: lấy một event, dispatch hoàn tất state handler/transition rồi mới nhận event tiếp theo.
- Queue của AO dùng kernel queue và do caller cấp mảng pointer storage.
- AO task start cùng lúc với create; state machine được start trước vòng nhận event.
- Dynamic event được release sau mỗi dispatch; static event vẫn do caller sở hữu.
- v1 dùng one-task-per-AO, không có shared executor.

<a id="debug"></a>
## Debug và failure modes

- AO không dispatch: kiểm tra dedicated hairtos task, AO queue và start order.
- Ping-pong dừng: kiểm tra post status, queue capacity và RTC handler return.
- Dynamic event ownership phải được release sau dispatch; static event vẫn caller-owned.
- Priority của AO đi qua scheduler hairtos như task bình thường.

<a id="validation"></a>
## Validation

- Example là target-only trong CMake; host evidence không thay thế ARM cross-build, OpenOCD và hardware validation.
- Host validation baseline: `make TARGET=bluepill_f103c8 host-tests` PASS toàn bộ suite.

### Lệnh chuẩn

```bash
make TARGET=bluepill_f103c8 EXAMPLE=13-02-active-object build
make TARGET=bluepill_f103c8 EXAMPLE=13-02-active-object run
make TARGET=bluepill_f103c8 EXAMPLE=13-02-active-object check
```

<a id="source-map"></a>
## Source map và references

- `examples/13-02-active-object/main.c`
- `cmake/hairtos_examples.cmake`
- `haievent/src/he_active.c`
- `haievent/internal/he_internal.h`
- `haievent/src/he_state_machine.c`
- `kernel/src/hr_queue.c`

### Tài liệu tham khảo


**Nguồn implementation trong repository:**
- `haievent/src/he_active.c`
- `haievent/internal/he_internal.h`
- `haievent/src/he_state_machine.c`
- `kernel/src/hr_queue.c`
