# Publish/Subscribe

## 1. Mục tiêu

Cho phép một event được chuyển tới nhiều Active Object mà publisher không biết trực tiếp từng consumer.

## 2. Subscription table

Application cung cấp mảng `he_active_t *` có kích thước:

```text
signal_count * max_subscribers_per_signal
```

Mỗi signal có một vùng slot cố định.

## 3. Subscribe và unsubscribe

Chỉ user signal (`>= HE_SIG_USER`) được đăng ký. Duplicate subscription trả invalid state. Unsubscribe compact các slot còn lại.

## 4. Publish

Framework snapshot subscriber list trong critical section rồi post ngoài critical section. Điều này giảm thời gian mask interrupt và tránh list thay đổi giữa vòng lặp.

## 5. Dynamic event ownership

Mỗi successful shared post giữ một reference. Sau khi thử tất cả subscriber, publisher reference được release. Event trở về pool khi AO cuối cùng dispatch xong.

## 6. Partial delivery

API trả overall status đầu tiên gặp lỗi và optional delivered count. Một số subscriber có thể nhận trong khi subscriber khác full.

## 7. Invariants

- Subscriber phải là AO valid.
- Signal trong range table.
- Snapshot không vượt `HE_CFG_MAX_ACTIVE_OBJECTS`.
- Static event lifetime do caller quản lý.

## 8. Giới hạn

Không có wildcard subscription, topic string hoặc retained last value.
