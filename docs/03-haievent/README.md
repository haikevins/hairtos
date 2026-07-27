# 03 — Framework haievent

## 1. Mục tiêu

Nhóm này mô tả framework event-driven tùy chọn chạy trên public API của `hairtos`.

## 2. Nội dung

- [event-model.md](event-model.md)
- [state-machine.md](state-machine.md)
- [active-object.md](active-object.md)
- [time-event.md](time-event.md)
- [publish-subscribe.md](publish-subscribe.md)

## 3. Dependency

`haievent` chỉ include public kernel API; framework không truy cập `kernel/internal`, board hoặc SoC register.

## 4. Portability

Khi kernel public contract hoạt động trên target mới, phần lớn `haievent` được giữ nguyên. Time event yêu cầu software timer và tick backend của target hoạt động đúng.

## 5. Kiểm thử

Host tests kiểm tra event ownership, state transitions, queueing và publish/subscribe. Target examples 13-01 đến 13-06 kiểm tra integration với scheduler và timer.
