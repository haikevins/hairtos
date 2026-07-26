# Roadmap

## 1. Các mốc phát triển đã hoàn thành

| Phase | Chủ đề | Trạng thái |
|---:|---|---|
| 0 | Specification | ✅ Hoàn thành |
| 1 | Bare-metal foundation | ✅ Hoàn thành |
| 2 | Intrusive list và kernel data structures | ✅ Hoàn thành |
| 3 | TCB và initial task stack | ✅ Hoàn thành |
| 4 | Start first task bằng SVC | ✅ Hoàn thành |
| 5 | PendSV cooperative context switch | ✅ Hoàn thành |
| 6 | Priority scheduler | ✅ Hoàn thành |
| 7 | SysTick và delay | ✅ Hoàn thành |
| 8 | Preemption và round-robin | ✅ Hoàn thành |
| 9 | Queue và blocking | ✅ Hoàn thành |
| 10 | Semaphore và mutex | ✅ Hoàn thành |
| 11 | Suspend/resume | ✅ Hoàn thành |
| 12 | Software timer | ✅ Hoàn thành |
| 13 | haievent framework | ✅ Hoàn thành |
| 14 | Memory allocator lab | ✅ Hoàn thành |
| 15 | Kernel benchmark | ✅ Hoàn thành |
| 16 | Diagnostics và stabilization | ✅ Hoàn thành |

## 2. Quan hệ giữa các mốc

- Phase 0–3 tạo nền tảng memory, data structure và initial context.
- Phase 4–8 hoàn thiện task startup, context switch, scheduler, tick và preemption.
- Phase 9–12 bổ sung IPC, synchronization, task administration và timer service.
- Phase 13 xây framework event-driven phía trên kernel.
- Các mốc cuối tập trung allocator experiment, benchmark, diagnostics và stabilization.

## 3. Trạng thái hiện tại

Repository hiện là **hairtos mainline**. Bảng trên chỉ ghi lại lộ trình phát triển; tên phase không được dùng làm tên release hoặc artifact chính; riêng example tích hợp vẫn giữ tên lịch sử `16-diagnostics-stress-stabilization` để khớp roadmap. “Hoàn thành” nghĩa là source và validation hiện diện trong repository; không có nghĩa mọi example đã được chạy trên mọi Blue Pill clone hoặc mọi toolchain.

## 4. Lịch sử chi tiết

Roadmap chỉ mô tả tiến độ ở mức cao; thiết kế hiện hành nằm trong các nhóm tài liệu subsystem tương ứng.
