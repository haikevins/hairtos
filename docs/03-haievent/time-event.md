# Time event

## 1. Mục tiêu

Chuyển software timer expiration thành event gửi tới Active Object.

## 2. Thành phần

Time event chứa hairtos timer, target AO, static timeout event và dropped counter.

## 3. Luồng

```text
SysTick
  -> timer pending
  -> timer-service callback
  -> post HE_SIG_TIMEOUT/custom signal
  -> AO task dispatch
```

State handler không chạy trong timer-service callback; callback chỉ post.

## 4. API

- create static;
- arm;
- disarm;
- rearm;
- change period;
- query armed/dropped count.

## 5. Dropped event

Nếu AO queue không nhận được event khi callback post non-blocking, dropped counter tăng. Application có thể dùng counter để phát hiện queue capacity hoặc priority không phù hợp.

## 6. Periodic và one-shot

`periodic` được ánh xạ sang auto-reload timer. Signal và period được cố định khi create nhưng period có thể đổi qua API.

## 7. Giới hạn

Không tạo dynamic event cho mỗi tick; cùng static event được tái sử dụng nên không được giữ pointer sau dispatch.
