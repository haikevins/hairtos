# Kiến trúc tổng thể

## 1. Mục tiêu

hairtos là RTOS static-first dành cho vi điều khiển nhỏ. Bản hiện tại chạy trên STM32F103C8T6/Cortex-M3 và cung cấp task, fixed-priority scheduler, timeout, queue, semaphore, mutex có priority inheritance, software timer, diagnostics và framework HairEvent.

## 2. Các lớp

```text
Application / Examples
        |
        +--> HairEvent framework
        |       |
        |       +--> hairtos public API
        |
        +------> hairtos public API
                        |
                  Kernel internals
                        |
                 Cortex-M3 port
                        |
         Board / drivers / STM32F1 SoC
```

### Application

Chứa task entry, state handlers, static storage và logic sản phẩm. Application không được truy cập `kernel/internal/`.

### HairEvent

Là lớp tùy chọn phía trên kernel, gồm event pool, Active Object, flat state machine, time event và publish/subscribe. HairEvent chỉ gọi public API của hairtos.

### Kernel public API

Nằm trong `kernel/include/hairtos/`. Đây là hợp đồng ổn định dành cho application và framework.

### Kernel internals

Nằm trong `kernel/internal/` và `kernel/src/`. Bao gồm intrusive list, TCB thật, ready queues, wait lists, timeout lists và scheduler policy.

### Port và platform

`arch/arm/cortex-m3/` xử lý PSP/MSP, SVC, PendSV, critical section và fault capture. `soc/`, `boards/` và `drivers/` cung cấp startup, clock, GPIO, UART và timer phần cứng.

## 3. Quyết định thiết kế chính

- Static allocation là mặc định; kernel không dùng `malloc`.
- Priority `0` là cao nhất; priority cuối được dành cho idle task.
- SysTick mặc định 1 kHz.
- PendSV là điểm duy nhất thực hiện context switch sau khi kernel đã chạy.
- Task context sử dụng PSP; exception sử dụng MSP.
- IPC blocking sử dụng wait list ưu tiên và timeout list chung.
- Callback software timer chạy trong timer-service task, không chạy trong SysTick ISR.
- HairEvent dispatch theo run-to-completion.

## 4. Ranh giới chức năng

Memory allocator ở Phase 14 là lab độc lập và không cấp phát TCB, queue hay timer. Kernel benchmark chỉ được link trong example Phase 15. Diagnostics nâng cao chỉ được bật khi macro cấu hình tương ứng được định nghĩa.

## 5. Source liên quan

```text
kernel/include/hairtos/
kernel/internal/
kernel/src/
hairevent/
arch/arm/cortex-m3/
soc/stm32f1/
boards/bluepill_f103c8/
drivers/
```

## 6. Giới hạn hiện tại

- Chỉ single-core.
- Không FPU context và không MPU isolation.
- Không dynamic task creation/deletion.
- HairEvent chỉ có flat state machine, chưa có hierarchical state machine.
- Chưa có port context-switch hoàn chỉnh cho Cortex-M0.
