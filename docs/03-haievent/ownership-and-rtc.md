# Event ownership và run-to-completion

## Ownership là contract quan trọng nhất

Dynamic event có reference count. Mọi API post/publish phải xác định ai đang sở hữu reference.

### Direct post

Caller tạo dynamic event với ref=1. Khi post thành công, ownership được chuyển vào AO queue. AO release sau dispatch.

Nếu post thất bại, caller phải xử lý theo API contract và không được giả định event đã biến mất.

### Publish

Một event có thể tới nhiều AO:

```text
publisher ref
 + retain cho subscriber A
 + retain cho subscriber B
 ...
 -> publisher release
 -> mỗi AO release sau dispatch
 -> ref 0 -> pool
```

Partial delivery có thể xảy ra nếu một queue không nhận event.

## Static event

Không dùng reference count để free. Caller phải bảo đảm event còn tồn tại tới khi mọi consumer dùng xong.

## RTC

Một AO không dispatch event B trong khi handler của event A chưa return.

State handler nên:

- tính toán ngắn;
- transition/post/publish;
- không `hr_task_delay`;
- không wait queue/semaphore forever;
- không giữ mutex lâu.

## Vì sao blocking trong state handler xấu?

Một AO bị block giữa dispatch làm queue của chính nó ngừng tiêu thụ; event latency tăng và logic "một event -> một reaction hoàn chỉnh" bị phá.

## V1 limitation

Framework chưa có dispatch-context flag để kernel từ chối blocking API. Đây là v2 priority: debug build có thể assert nếu state handler gọi blocking operation.

## Reentrancy

Không dispatch reentrant cùng state machine. Cross-AO communication dùng queue/post thay vì gọi state handler của AO khác trực tiếp.
