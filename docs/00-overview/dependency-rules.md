# Quy tắc dependency

## 1. Mục tiêu

Giữ kernel có thể kiểm thử trên host, ngăn application phụ thuộc vào layout nội bộ và tránh cycle giữa framework, kernel và platform.

## 2. Hướng dependency hợp lệ

```text
Application -> haievent -> hairtos public API -> internal kernel -> port
Application ----------------^                       |
Board/drivers --------------------------------------+
```

## 3. Quy tắc bắt buộc

1. Application không include file trong `kernel/internal/`.
2. haievent không include `hr_port.h`; nó đi qua `hr_context.h`.
3. Kernel generic không đọc thanh ghi STM32 trực tiếp.
4. Port không phụ thuộc vào application hoặc haievent.
5. Driver không gọi scheduler internals.
6. Lab allocator không trở thành dependency của kernel runtime.
7. Benchmark source chỉ được link khi chọn benchmark example.
8. Public header không expose internal control block.

## 4. Critical section abstraction

Framework dùng:

```c
hr_irq_state_t state = hr_critical_enter();
/* cập nhật dữ liệu dùng chung */
hr_critical_exit(state);
```

ISR đánh thức task dùng `hr_yield_from_isr(required)` thay vì gọi trực tiếp PendSV register.

## 5. Lý do

- Host tests có thể mock port.
- Có thể thay MCU mà không sửa scheduler.
- ABI public không bị khóa vào layout TCB.
- Fault và context-switch code vẫn được cô lập tại architecture layer.

## 6. Kiểm tra

`make repository-hygiene-check` phải thất bại nếu skeleton/ghost API quay lại. Review code cần kiểm tra include graph, source list của Make/CMake và public/private boundary.
