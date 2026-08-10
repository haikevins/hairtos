# Nguyên tắc thiết kế

## 1. Static-first

Kernel object phải có lifetime và memory footprint biết trước. Public opaque object cho phép application cấp storage mà không phụ thuộc layout internal.

Không biến allocator lab thành dependency ngầm của kernel.

## 2. Generic policy, target-specific mechanism

Scheduler policy, wait ordering, timeout logic, queue, semaphore và mutex thuộc kernel generic.

Stack frame, context switch, interrupt masking, fault entry, startup, linker, pins và peripheral clock thuộc port/SoC/board/driver.

## 3. Một nguồn sự thật cho build

CMake quyết định target/example/module/source. Makefile chỉ wrap UX.

Không lặp lại source mapping trong nhiều build system.

## 4. Public API nhỏ

Application dùng `hairtos/`, `haievent/` và `board.h`. Internal header được coi là implementation detail.

Opaque public storage đổi layout internal mà không buộc application truy cập field.

## 5. Invariant trước feature count

Một feature chỉ có giá trị khi state transitions và ownership rõ. Vì vậy direct handoff, wait cleanup, timeout race và list validation quan trọng hơn việc có thật nhiều API.

## 6. ISR không block

ISR API phải nonblocking. ISR chỉ update object, wake task và request deferred switch.

Application callback không chạy trực tiếp từ SysTick.

## 7. Event ownership phải xác định

Dynamic event có owner/reference count. Mọi path success/failure phải biết ai release.

Static event có lifetime do caller đảm bảo.

## 8. Run-to-completion cho state handler

State handler nên xử lý nhanh, không delay, không chờ semaphore/queue forever và không giữ mutex lâu. v1 mới quy định bằng architecture; v2 dự kiến enforce rõ hơn.

## 9. Portability phải được chứng minh

Tách folder chưa đủ. Một abstraction chỉ được coi là tốt sau khi target thứ hai dùng lại kernel/framework mà không sửa chúng.

## 10. Diagnostics là một phần của kernel

Stack guard, invariant check và panic record không phải "debug print thêm". Chúng là cơ chế giúp phát hiện khi assumptions của scheduler/memory bị phá.

## 11. Version 2 không được biến thành rewrite vô hạn

Mọi thay đổi v2 phải trả lời ít nhất một trong các mục tiêu:

- tăng tính đúng;
- giảm latency/power;
- tăng portability;
- tăng observability;
- mở rộng event modeling có kiểm soát;
- tăng khả năng kiểm thử.

Feature không phục vụ các mục tiêu này nên để sau 2.0.
