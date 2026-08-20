# Glossary

[← Root README](../../README.md) · [↑ Back to section](README.md) · [Next →](known-limitations.md)

| Thuật ngữ | Nghĩa trong **hairtos** |
| --- | --- |
| **AO** | Active Object = dedicated hairtos task + event-pointer queue + flat state machine. |
| **Base priority** | Priority cấu hình khi tạo task. |
| **Effective priority** | Priority scheduler đang dùng sau priority inheritance. |
| **Ready set** | Tập 8 FIFO intrusive queues + bitmap cho task READY. |
| **Wait list** | Priority-ordered intrusive list của task chờ object. |
| **Timeout list** | Hai sorted lists current/overflow cho deadline wrap-aware. |
| **Direct handoff** | Hoàn tất data/token/ownership trực tiếp cho waiter thay vì buộc qua buffered state trung gian. |
| **RTC** | Run-to-completion: AO xử lý xong một event trước khi lấy event kế. |
| **FSM** | Flat state machine của haievent v1. |
| **HSM** | Hierarchical State Machine; chưa implemented v1. |
| **MSP** | Main Stack Pointer; bootstrap/handler stack. |
| **PSP** | Process Stack Pointer; task Thread-mode stack sau SVC. |
| **SVC** | Supervisor Call dùng để start first task. |
| **PendSV** | Lowest-priority deferred exception dùng context switch. |
| **PRIMASK** | Cortex-M interrupt mask dùng bởi critical section v1. |
| **TCB** | Internal Task Control Block nằm trong opaque `hr_task_t` storage. |
| **Opaque object** | Public aligned byte storage che internal control-block layout. |
| **Static-first** | Caller cấp storage trước; kernel không tự heap allocate object runtime. |
| **Event pool** | Fixed-block pool cho dynamic `he_event_t`. |
| **Reference count** | Số owner/reference active của dynamic event. |
| **Time Event** | Adapter kernel software timer → timeout event post vào AO. |
| **Timer-service task** | Task chạy user software-timer callback ngoài ISR. |
| **DWT CYCCNT** | Cortex-M cycle counter dùng benchmark clock target. |
| **Retained record** | Panic/fault record đặt `.noinit` để tồn tại qua reset phù hợp. |
| **Target manifest** | CMake file bind architecture/SoC/board/driver/linker/debug cho target. |
| **Host test** | Native build với mock port + ASan/UBSan cho generic logic. |

## Lưu ý

Các từ như “priority”, “timer”, “event” có nghĩa cụ thể theo implementation này. Ví dụ software timer callback **không** chạy trong SysTick ISR; “dynamic event” dùng fixed-block pool chứ không đồng nghĩa general heap allocation.
