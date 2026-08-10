# Phân tích toàn bộ project hairtos v1

## 1. Phạm vi audit

Audit này đọc toàn bộ source C/header/assembly, CMake/Makefile, linker/startup, target manifest, tests, examples và tài liệu của `hairtos 1.0.0-rc1`.

Quy mô source chính tại baseline:

| Nhóm | Xấp xỉ dòng |
|---|---:|
| `kernel/` | 5.910 |
| `haievent/` | 1.540 |
| `tests/` | 2.791 |
| `examples/` | 3.820 dòng code, chưa tính README |
| `labs/` | 1.148 code |
| `arch/` | 526 |
| `soc/` | 559 |
| `drivers/` | 468 |
| `boards/` | 394 |
| `benchmarks/` | 237 code |

Đây là một codebase đủ lớn để thể hiện RTOS thực, nhưng vẫn nhỏ hơn nhiều so với RTOS production, nên rất phù hợp để audit từ đầu đến cuối.

## 2. Kết luận kiến trúc

`hairtos` thực sự gồm ba tầng runtime:

```text
Preemptive RTOS Kernel
        |
        +--> haievent Event Framework
                    |
                    +--> State Machine / Active Object
```

Kernel không phụ thuộc `haievent`. `haievent` sử dụng public task/queue/timer/context APIs của kernel. Active Object hiện được ánh xạ 1:1 thành một RTOS task.

## 3. Kernel

Kernel đã có đầy đủ các khối cơ bản của RTOS fixed-priority:

- lifecycle RESET → INITIALIZED → RUNNING → PANIC;
- static task creation;
- ready queues theo priority + bitmap;
- SVC first-task startup và PendSV context switch trên Cortex-M3;
- preemption + time slicing;
- delay/timeout;
- queue/semaphore/mutex;
- priority inheritance;
- suspend/resume;
- software timer;
- diagnostics.

Điểm đáng chú ý là task RUNNING vẫn nằm trong ready queue. Scheduler selector dựa trên reason (`YIELD`, `BLOCK`, `PREEMPT`, `TIME_SLICE`) để quyết định rotate/remove/state transition. Vì vậy mọi thay đổi scheduler phải giữ invariant này.

## 4. Intrusive structures

Một TCB có node riêng cho:

```text
ready
wait
timeout
all-task
owned-mutex relationships
```

Cách này tránh allocation node động và cho phép một task đồng thời tồn tại trong các cấu trúc logic khác nhau bằng các node khác nhau. Node không được insert vào hai list cùng lúc; list API lưu pointer về owning list để bắt double insert/remove.

## 5. Task và stack

Public `hr_task_t` là opaque static storage. Internal TCB giữ saved stack pointer tại field đầu tiên; assembly phụ thuộc offset này.

Stack:

- type `uint32_t`;
- căn 8 byte theo Cortex-M3 port;
- fill pattern `0xA5`;
- guard `0xDEADBEEF`;
- initial frame chứa software-saved R4–R11 và hardware-compatible R0–R3/R12/LR/PC/xPSR;
- R0 mang argument;
- task return hiện rơi vào `hr_task_exit_error()` và spin vô hạn.

Task-return spin là một điểm cần cải thiện ở v2: nên chuyển thành controlled panic/hook hoặc policy được cấu hình.

## 6. Scheduler

Scheduler là fixed-priority, priority 0 cao nhất. Mỗi priority có FIFO list; bitmap cho biết priority nào non-empty.

Độ phức tạp chọn priority hiện tại quét tối đa `HR_CFG_PRIORITY_COUNT` (mặc định 8, tối đa 32), nên đủ nhỏ và deterministic. Không có EDF/deadline scheduling hoặc dynamic application priority API.

Round-robin xảy ra khi quantum hết và có peer cùng priority. Preemption xảy ra khi một task priority cao hơn trở READY.

## 7. Time và timeout

Tick là `uint32_t`, mặc định 1 kHz. Timeout dùng hai ordered lists: current epoch và overflow epoch. Khi tick wrap, lists hoán đổi.

Thiết kế này tránh so sánh absolute time sai qua wrap. `delay_until()` hỗ trợ periodic release và giảm drift.

Kernel còn xử lý race quan trọng: task vừa block nhưng timeout một tick có thể hết trước PendSV. Selector không giả định task chắc chắn vẫn BLOCKED.

## 8. Queue

Queue là ring buffer caller-owned:

- fixed item size;
- fixed capacity;
- sender/receiver wait lists theo effective priority;
- direct handoff sender → blocked receiver;
- receive từ queue đầy có thể refill slot trực tiếp từ blocked sender;
- finite timeout và `HR_WAIT_FOREVER`;
- ISR send/receive nonblocking.

Direct handoff là điểm thiết kế tốt vì resource/data ownership được quyết định trước khi waiter thực sự chạy, giảm race "được wake nhưng tài nguyên đã bị task khác lấy".

## 9. Semaphore

Counting/binary semaphore không có ownership. `give()` ưu tiên handoff trực tiếp cho waiter trước khi tăng count. ISR give được hỗ trợ; blocking take từ ISR bị cấm.

## 10. Mutex

Mutex có owner, recursion count và waiters priority-ordered. Priority inheritance được tính lại từ base priority và highest waiter trên toàn bộ mutex task đang giữ.

Nếu owner đang chờ mutex khác, boost được propagate theo chuỗi; recursion depth được chặn bởi `HR_CFG_MAX_TASKS` để tránh vòng lặp do corruption/cycle.

Chưa có deadlock detection hoặc priority ceiling.

## 11. Suspend/resume

Suspend là administrative overlay, không đồng nghĩa hủy operation blocking. Một task BLOCKED có thể bị suspend; timeout/event vẫn có thể complete trong khi task suspended. Khi đó resume state được chuyển thành READY nhưng task chưa được enqueue cho tới `resume()`.

Đây là behavior tinh tế và đã tách đúng khỏi wait reason.

## 12. Software timer

Timer system dùng:

- ordered timeout list;
- pending callback list;
- binary wake semaphore;
- timer-service task;
- static service stack.

SysTick chỉ đánh dấu expiration/pending; callback chạy trong task context. Điều này tránh application callback trong interrupt context.

## 13. Diagnostics

Diagnostics v1 có ba tầng:

1. runtime counters;
2. health check kernel/list/task/stack;
3. retained panic/fault record.

Panic record lưu reason, tick, task, source hash/line và Cortex-M fault context. Section `.noinit.hairtos` cho phép giữ record qua một số loại reset.

Điểm còn thiếu là timeline/trace: v1 biết "điều gì cuối cùng hỏng" và tổng counters, nhưng chưa có ring buffer lịch sử event/switch dẫn đến lỗi.

## 14. Interrupt model

Cortex-M3 port dùng PRIMASK để critical section mask toàn bộ configurable interrupts. Cách này đơn giản và đúng cho kernel nhỏ, nhưng làm tăng interrupt latency khi critical section dài.

v2 nên xem xét BASEPRI + configurable max-syscall interrupt priority trên Cortex-M3/M4 để high-urgency ISR có thể tiếp tục chạy.

## 15. haievent event model

Dynamic event lấy từ fixed block pool và bắt đầu với reference count 1. Static event không thuộc pool.

Reference counting cho phép một dynamic event được publish tới nhiều subscriber. Block chỉ quay về pool khi reference cuối cùng release.

Hiện chưa có global size-class allocator; application tự chọn pool/block size.

## 16. Flat State Machine

FSM v1 hỗ trợ:

- initial/current/target handler;
- `HANDLED`, `IGNORED`, `TRANSITION`;
- ENTRY/EXIT/INIT;
- init transition chain với guard `HE_CFG_MAX_INIT_TRANSITIONS`.

Nó chưa có parent state, superstate, history hoặc event propagation. `HE_CFG_ENABLE_HIERARCHICAL_SM` đang bằng 0.

## 17. Active Object

Một Active Object chứa:

```text
1 RTOS task
1 RTOS queue
1 state machine
1 stack
1 priority
1 user context
```

Task loop:

```text
receive event
→ dispatch state machine
→ release event
→ receive next event
```

Đây là RTC về mặt cấu trúc event loop. Tuy nhiên kernel không cấm state handler gọi blocking API, nên RTC hiện là contract kiến trúc chứ chưa được runtime-enforce.

Nếu dispatch/event validity lỗi, AO loop hiện yield/spin thay vì tích hợp diagnostics panic — đây là điểm nên sửa v2.

## 18. Time Event và Pub/Sub

Time Event wrap software timer; callback chỉ post static timeout event vào AO. Nếu queue full, dropped counter tăng.

Pub/Sub snapshot subscriber list trong critical section rồi post ngoài critical section. Dynamic event được retain theo từng successful delivery và publisher reference được release ở cuối. Partial delivery có thể xảy ra.

## 19. Portability

V1 đã cải thiện mạnh portability:

```text
arch/       CPU contract
soc/        MCU-family startup/clock/register
boards/     board/linker/pins
drivers/    public API + SoC backend
cmake/targets/ target binding
```

`CMakeLists.txt` không hard-code STM32F1. Target discovery tự động qua manifest.

Nhưng mới chỉ có một target hoàn chỉnh. Một abstraction chưa được target thứ hai chứng minh vẫn có khả năng chứa assumption ẩn.

Một số educational examples còn cố ý dùng STM32F1 EXTI register trực tiếp. Đây không ảnh hưởng kernel portability nhưng làm example đó không tự động portable.

## 20. Build system

CMake là source-of-truth:

```text
example -> modules
module -> source
target -> hardware implementation
```

Makefile chỉ wrap configure/build/run/debug. Public/internal include scope được tách theo object libraries.

Đây là cấu trúc tốt hơn việc duy trì song song source mapping trong Make và CMake.

## 21. Test architecture

Tests gồm:

- host unit tests;
- mock port;
- Cortex-M0 compile probe;
- deterministic scheduler stress;
- target examples;
- allocator tests;
- benchmark statistics tests.

Trong môi trường audit hiện tại, toàn bộ 21 target example cross-build bằng Clang/LLD. Test binary host cho kết quả 64/64 PASS khi ASan runtime được preload đúng; lời gọi CTest mặc định tại môi trường này gặp loader-order warning của ASan trước khi test body chạy. Đây là vấn đề môi trường loader, không phải kết quả test logic.

Hardware flash/runtime chưa được thực hiện trong audit này.

## 22. Điểm mạnh

- static-first nhất quán;
- public/internal boundary rõ;
- scheduler/timeout/wait state machine có invariant;
- direct handoff trong IPC;
- chained priority inheritance;
- retained diagnostics;
- host-testable generic C;
- target manifest;
- `haievent` thực sự chạy trên kernel thay vì là demo tách rời;
- example roadmap giúp kiểm chứng incremental.

## 23. Giới hạn v1

- chỉ single-core;
- không FPU context;
- không MPU isolation;
- PRIMASK critical section;
- không tickless idle;
- không task deletion;
- không general kernel heap;
- không deadlock diagnostics;
- flat FSM;
- không deferred event;
- RTC chưa được cưỡng chế;
- mỗi AO tốn một full RTOS stack;
- chưa có trace timeline;
- mới một real target;
- một số examples còn target-specific.

## 24. Hướng Version 2

Version 2 nên ưu tiên những khoảng trống làm tăng **tính đúng, portability và khả năng debug**, thay vì tăng số API bằng mọi giá:

```text
Port contract v2
→ Second real target
→ Interrupt priority contract
→ HSM
→ RTC enforcement / deferred events
→ Tickless low-power
→ Trace/observability
→ Strong CI + hardware validation
```

Chi tiết: [`../09-version2/README.md`](../09-version2/README.md).
