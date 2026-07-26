# Event model

## 1. Mục tiêu

Biểu diễn thông điệp bằng signal và payload mở rộng, hỗ trợ static event và dynamic event từ fixed-block pool.

## 2. Signal

`he_signal_t` là `uint16_t`. Signal 0–4 dành cho framework; user signal bắt đầu từ `HE_SIG_USER` (32).

## 3. Event header

`he_event_t` chứa pool pointer, magic, size, reference count, signal và flags. Payload application có thể được đặt trong struct mở rộng với `he_event_t` ở đầu.

## 4. Static event

`he_event_init_static()` tạo event không thuộc pool. Static event không bị giải phóng và caller phải bảo đảm lifetime đủ dài cho mọi consumer.

## 5. Dynamic event

`he_event_new()` lấy một block từ pool. Block size phải đủ cho event struct. Reference count theo dõi số consumer đang sở hữu.

## 6. Ownership

- Direct `he_active_post()` chuyển ownership của dynamic event sang AO khi thành công.
- Failed post không được giả định đã giải phóng; caller xử lý theo contract của API/source.
- Publish giữ một reference cho mỗi subscriber thành công và cuối cùng release ownership của publisher.
- AO release event sau dispatch.

## 7. Event pool

Pool dùng fixed-size free list, không fragmentation. API cung cấp free count và block count để theo dõi exhaustion.

## 8. Giới hạn

Không có nhiều pool theo size class tự động; application phải chọn block size/capacity phù hợp.
