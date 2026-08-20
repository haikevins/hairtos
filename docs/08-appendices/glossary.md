# Glossary

[← Root README](../../README.md) · [↑ Back to section](README.md) · [Next →](known-limitations.md)

| Term | Meaning in **hairtos** |
| --- | --- |
| **AO** | Active Object = dedicated hairtos task + event-pointer queue + flat state machine. |
| **Base priority** | Priority configured when a task is created. |
| **Effective priority** | Priority currently used by the scheduler after priority inheritance. |
| **Ready set** | Eight FIFO intrusive queues plus a bitmap for READY tasks. |
| **Wait list** | Priority-ordered intrusive list of tasks waiting on an object. |
| **Timeout list** | Two sorted current/overflow lists for wrap-aware deadlines. |
| **Direct handoff** | Completes data/token/ownership transfer directly to a waiter instead of forcing the transfer through intermediate buffered state. |
| **RTC** | Run-to-completion: an AO completes processing one event before dequeuing the next. |
| **FSM** | The flat state machine implemented by haievent v1. |
| **HSM** | Hierarchical State Machine; not implemented in v1. |
| **MSP** | Main Stack Pointer; bootstrap/handler stack. |
| **PSP** | Process Stack Pointer; task Thread-mode stack after SVC. |
| **SVC** | Supervisor Call used to start the first task. |
| **PendSV** | Lowest-priority deferred exception used for context switching. |
| **PRIMASK** | Cortex-M interrupt mask used by v1 critical sections. |
| **TCB** | Internal Task Control Block stored inside opaque `hr_task_t` storage. |
| **Opaque object** | Public aligned byte storage che internal control-block layout. |
| **Static-first** | The caller provides storage up front; the kernel does not heap-allocate runtime objects itself. |
| **Event pool** | Fixed-block pool for dynamic `he_event_t`. |
| **Reference count** | Number of active owners/references to a dynamic event. |
| **Time Event** | Adapter from a kernel software timer to a timeout event posted to an AO. |
| **Timer-service task** | Task that executes user software-timer callbacks outside ISR context. |
| **DWT CYCCNT** | Cortex-M cycle counter used by the target benchmark clock. |
| **Retained record** | Panic/fault record placed in `.noinit` so it survives a suitable reset. |
| **Target manifest** | CMake file that binds architecture/SoC/board/driver/linker/debug selections for a target. |
| **Host test** | Native build with a mock port plus ASan/UBSan for generic logic. |

## Notes

Terms such as “priority”, “timer”, and “event” have implementation-specific meanings here. For example, software-timer callbacks **do not** execute in the SysTick ISR, and a “dynamic event” comes from a fixed-block pool rather than implying general-purpose heap allocation.
