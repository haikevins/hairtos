# Quy tắc dependency

## Hướng hợp lệ

```text
application -> haievent -> hairtos public -> kernel internal -> arch
application -> hairtos public
application -> board public
board -> drivers/SoC
target manifest -> binds all target-specific implementation
```

## Các dependency bị cấm

- `kernel/src` include STM32 register headers.
- `haievent/src` include `kernel/internal`.
- application bình thường include `kernel/internal`.
- driver implementation gọi scheduler internals.
- kernel generic gọi `board_*`.
- SoC layer include application.
- allocator lab trở thành kernel allocation backend ngầm.

## Ngoại lệ có chủ đích

Example 15 benchmark có thể truy cập scheduler internal để đo policy path. Host tests cần internal layout để kiểm tra invariant. Những exception này phải được CMake cấp include riêng, không mở internal include cho mọi source.

## Lý do

Dependency một chiều cho phép:

- unit test generic C trên host;
- port MCU mà không sửa scheduler;
- thay board mà không sửa driver contract;
- thay internal TCB layout mà không sửa application;
- kiểm tra include boundary bằng compiler.

## Critical/ISR boundary

Framework hoặc generic component dùng:

```c
hr_irq_state_t state = hr_critical_enter();
/* short atomic update */
hr_critical_exit(state);
```

Không gọi Cortex-M PRIMASK trực tiếp ngoài architecture port.

ISR wake task dùng ISR-safe API + `hr_yield_from_isr()`.

## Review checklist

Một PR thêm dependency mới cần kiểm tra:

- include direction;
- CMake include visibility;
- target assumptions;
- blocking/ISR context;
- ownership/lifetime;
- tests.
