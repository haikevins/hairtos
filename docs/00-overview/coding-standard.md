# Coding standard

## 1. Mục tiêu

Quy tắc này ưu tiên tính xác định, khả năng audit và phát hiện lỗi tại compile time.

## 2. Ngôn ngữ và compiler flags

- C11 và GNU assembly cho Cortex-M3.
- `-Wall -Wextra -Werror -Wshadow -Wundef`.
- Bật cảnh báo conversion/sign-conversion.
- Target dùng freestanding, không giả định libc đầy đủ.

## 3. Naming

| Thành phần | Prefix |
|---|---|
| Public hairtos | `hr_` |
| Public haievent | `he_` |
| Configuration | `HR_CFG_`, `HE_CFG_` |
| Internal hairtos | `hr_*_internal` hoặc internal header |
| Board API | `board_` |

Kiểu public kết thúc bằng `_t`; enum constant viết hoa; function và variable dùng snake_case.

## 4. Memory

- Ưu tiên static storage.
- Không dùng recursion trong kernel hot path.
- Không dùng variable-length array.
- Kiểm tra overflow khi tính size/capacity.
- Opaque object phải có `_Static_assert` với internal layout.

## 5. Concurrency

- Mọi cập nhật ready/wait/timeout structure phải nằm trong critical section phù hợp.
- ISR API không block.
- PendSV là nơi thực hiện context switch.
- Callback timer và state handler không chạy trong SysTick ISR.

## 6. Error handling

- Public API trả `hr_status_t`.
- Invalid argument khác invalid state.
- Internal corruption dẫn tới `HR_ERROR_INTERNAL` hoặc panic theo ngữ cảnh.
- Không bỏ qua return value nếu operation có thể rollback cấu trúc intrusive.

## 7. Documentation và tests

Source mới phải có tài liệu subsystem hoặc API tương ứng. Mỗi bug về list/state phải có regression test. Không giữ file “pending” trong release.
