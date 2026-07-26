# Context switch trên Cortex-M3

## 1. Mục tiêu

Khởi chạy first task bằng SVC và chuyển task bằng PendSV, tận dụng hardware exception frame.

## 2. Stack frame

Hardware tự push/pop:

```text
R0 R1 R2 R3 R12 LR PC xPSR
```

hairtos assembly tự lưu/khôi phục:

```text
R4 R5 R6 R7 R8 R9 R10 R11
```

## 3. Start first task

```text
main dùng MSP
  -> hr_kernel_start
  -> prepare current TCB
  -> SVC
  -> reset MSP về _estack
  -> restore R4-R11 từ task stack
  -> set PSP và CONTROL.SPSEL
  -> exception return 0xFFFFFFFD
  -> task entry chạy bằng PSP
```

## 4. PendSV switch

```text
PendSV entry
  -> MRS current PSP
  -> STMDB R4-R11
  -> lưu SP vào TCB field 0
  -> gọi hr_kernel_select_next_from_pendsv()
  -> lấy SP TCB mới
  -> LDMIA R4-R11
  -> MSR PSP
  -> BX LR
```

Selector C chạy khi interrupt bị mask để cập nhật scheduler atomically.

## 5. MSP và PSP

- Handler mode luôn dùng MSP.
- Thread mode sau kernel start dùng PSP.
- Mỗi task có PSP riêng.

## 6. Exception priorities

PendSV được đặt priority thấp để context switch chỉ xảy ra sau ISR khác. SysTick tạo scheduling decision nhưng không trực tiếp thay stack.

## 7. Kiểm thử

- Host test kiểm tra initial stack layout.
- Phase 4 kiểm tra SVC.
- Phase 5 kiểm tra local variable được bảo toàn qua PendSV.
- Validation script kiểm tra strong symbols và disassembly.

## 8. Giới hạn

`HR_CFG_USE_FPU=0`; không lưu floating-point context mở rộng.
