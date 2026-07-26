# Diagnostics và fault analysis

## 1. Mục tiêu

Phát hiện kernel corruption, stack exhaustion và Cortex-M fault với dữ liệu đủ để debug sau reset.

## 2. Health report

Kiểm tra scheduler bitmap, task/list membership, timeout structure và stack guards. Report nêu current task, minimum free stack và số task dưới margin.

## 3. Runtime counters

Counters giúp phân biệt workload: yield, block, preemption, time slice, timeout wake và switch tổng.

## 4. Retained fault record

Fault handler lấy stacked PC/LR và SCB status. Record có signature/version/sequence/boot count để phân biệt dữ liệu hợp lệ.

## 5. Quy trình debug

1. Đọc panic reason và task name.
2. Dùng PC/LR tra `.elf` bằng `addr2line`.
3. Giải mã CFSR thành MemManage/BusFault/UsageFault sub-bits.
4. Kiểm tra BFAR/MMFAR valid bits.
5. Kiểm tra stack high-watermark và guard.
6. So sánh map file và source commit.

## 6. Fault injection

Example `hairtos` có macro tạo `udf #0`. Dùng để kiểm tra strong handler, `.noinit` và log sau reset.

## 7. Giới hạn

Power cycle có thể làm mất retained RAM. Record không thay thế trace timeline hoặc core dump đầy đủ.
