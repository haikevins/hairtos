# 01 — Lõi kernel

## 1. Mục tiêu

Nhóm này mô tả dữ liệu và luồng cốt lõi của kernel, độc lập board/SoC ở mức C generic.

## 2. Nội dung

- [memory-model.md](memory-model.md)
- [intrusive-data-structures.md](intrusive-data-structures.md)
- [task-model.md](task-model.md)
- [scheduler.md](scheduler.md)
- [kernel-lifecycle.md](kernel-lifecycle.md)
- [context-switch.md](context-switch.md)
- [interrupt-model.md](interrupt-model.md)
- [time-and-timeout.md](time-and-timeout.md)

## 3. Thứ tự đọc

Bắt đầu từ memory/data structures, tiếp theo task/scheduler/lifecycle, rồi context/interrupt/time.

## 4. Ranh giới port

Task state, ready/wait/timeout policy và IPC state machine thuộc kernel generic. Initial stack frame, critical section, ISR detection, first-task start và context switch thuộc architecture port qua `hr_port_*`.

## 5. Cấu hình port

`hr_port_config.h` cung cấp minimum stack, alignment, FPU-context capability và MPU capability. Kernel configuration kiểm tra dựa trên contract này thay vì giả định Cortex-M3 cố định.

## 6. Kiểm thử

Các thuật toán C được kiểm thử trên host; context switch/exception/tick cần cross-build và runtime validation trên target.
