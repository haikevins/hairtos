# Kiến trúc haievent

## Thành phần

```text
Event / Event Pool
        |
        v
Active Object Queue
        |
        v
Active Object Task
        |
        v
State Machine
        |
        +--> transition
        +--> publish
        +--> post
        +--> arm/disarm Time Event
```

## Tại sao cần framework khi đã có RTOS?

RTOS giải quyết *ai được CPU chạy* và *task chờ nhau thế nào*.

Framework giải quyết *application phản ứng với sự kiện gì* và *state hiện tại quyết định behavior nào*.

Thay vì nhiều task chứa vòng `if/switch` chia sẻ flags, application có thể biểu diễn behavior thành state handlers và event queues.

## Một AO = một task

v1 dùng dedicated task cho từng AO. Ưu điểm:

- priority rõ;
- isolation execution context;
- blocking receive queue đơn giản;
- tận dụng scheduler preemptive.

Nhược điểm:

- mỗi AO cần stack riêng;
- nhiều AO nhỏ có thể tốn RAM.

Shared executor là ý tưởng v2.x, không thay dedicated-task AO mặc định ở 2.0.

## RTC

AO loop dispatch đúng một event rồi mới lấy event tiếp theo. Tuy nhiên state handler vẫn có thể gọi blocking kernel API nếu developer cố tình làm. Vì thế RTC v1 là architecture contract, chưa phải runtime-enforced rule.

## Dependency

`haievent` dựa trên:

- task;
- queue;
- critical context wrapper;
- software timer.

Không biết board, SoC hoặc port assembly.
