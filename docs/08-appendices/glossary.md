# Glossary

| Thuật ngữ | Ý nghĩa trong hairtos |
|---|---|
| AO | Active Object = task + queue + FSM |
| Base priority | Priority application cấu hình |
| Effective priority | Priority sau mutex inheritance |
| Direct handoff | Hoàn tất data/token/ownership cho waiter trước khi wake |
| HSM | Hierarchical State Machine; chưa có ở v1 |
| ISR-safe | API nonblocking được thiết kế cho interrupt context |
| MSP | Main Stack Pointer, handler/bootstrap |
| PSP | Process Stack Pointer, task Thread mode trên Cortex-M |
| PendSV | Deferred context-switch exception trên Cortex-M |
| RTC | Run-to-completion event handling |
| SVC | Supervisor Call, dùng start first task ở current port |
| TCB | Task Control Block |
| Time slicing | Round-robin bằng quantum giữa task cùng priority |
| Timeout list | Ordered blocked-task deadlines |
| Wait list | Priority-ordered tasks chờ object |
| Watermark | Ước lượng stack free từ fill pattern |
| Static-first | Object memory do caller cung cấp, không kernel heap |
| Target manifest | CMake binding arch/SoC/board/driver/linker/debug |
| Retained record | Panic/fault data trong non-zeroed RAM section |
