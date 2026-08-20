# Coding standard của repository

> **Scope:** Convention quan sát từ source + compiler policy; không thay thế MISRA/CERT và repo không tuyên bố certification.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](capability-matrix.md) · [Next →](configuration.md)

## Mục lục

- [Compiler discipline](#compiler)
- [Naming/layers](#naming)
- [Types/conversions](#types)
- [Concurrency](#concurrency)
- [Error handling](#errors)
- [Data-structure discipline](#ds)
- [Documentation/test expectations](#docs)

<a id="compiler"></a>
## Compiler discipline

Target compile dùng C11, `-ffreestanding`, `-fno-common`, `-fno-builtin`, function/data sections và warning set strict: `-Wall -Wextra -Werror -Wshadow -Wundef -Wconversion -Wsign-conversion`. Host thêm `-pedantic` và ASan/UBSan.

<a id="naming"></a>
## Naming và layer

- `hr_` = hairtos kernel/public/internal symbol.
- `he_` = haievent.
- `board_` = board service.
- `stm32f1_` = SoC-specific helper.
- `HR_CFG_` / `HE_CFG_` = compile-time config.
- public header nằm trong include tree; internal header không được “tiện tay” expose.

<a id="types"></a>
## Types và conversion

Code dùng `<stdint.h>`, `<stddef.h>`, `size_t`, `uintptr_t` và suffix `U/UL` nhất quán. Warning conversion/sign-conversion ép mọi narrowing/unsigned interaction phải intentional. Pointer↔integer chỉ xuất hiện ở architecture/register/binary-boundary có lý do rõ.

<a id="concurrency"></a>
## Concurrency

- Critical section save/restore prior PRIMASK state, không đơn giản `enable irq` vô điều kiện.
- ISR path không gọi blocking API.
- Shared intrusive structure mutate dưới kernel/critical contract.
- User callback không chạy trong SysTick ISR.
- Mutex ownership và effective priority phải thay đổi atomically với wait/ready requeue.

<a id="errors"></a>
## Error handling

Public operation trả `hr_status_t` cho recoverable contract failure. Internal invariant/fault có assert/panic/diagnostics path. Examples dùng `board_panic()` để biến violation thành dễ debug thay vì tiếp tục với state không hợp lệ.

<a id="ds"></a>
## Data-structure discipline

Intrusive node init trước insert, double insert/remove bị reject, list có validator. Magic values phân biệt initialized opaque object. `_Static_assert` bảo đảm hidden control block fit public storage.

<a id="docs"></a>
## Documentation/test expectations

Feature mới cần public contract, source mapping, failure mode, host/target evidence phù hợp và update capability matrix. Version 2 proposal không được coi là implemented chỉ vì có header/doc.

## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)
