# Queue

## Model

Queue là fixed-capacity FIFO ring buffer. Application cung cấp byte storage, `item_size` và `capacity`.

Control block giữ head/tail/count và hai wait lists: sender, receiver.

## Send path

1. Nếu receiver đang block: copy trực tiếp item vào receiver buffer, complete receiver.
2. Nếu queue còn chỗ: copy vào tail, tăng count.
3. Nếu full + `HR_NO_WAIT`: `HR_ERROR_QUEUE_FULL`.
4. Nếu được block: sender lưu source buffer vào TCB, vào sender wait list và optional timeout.

## Receive path

1. Nếu queue có item: copy head ra caller.
2. Sau khi tạo slot, nếu sender đang block: copy item của sender vào slot vừa trống và wake sender.
3. Nếu queue empty nhưng sender waiter tồn tại theo state hợp lệ: có thể direct transfer.
4. Nếu empty + no-wait: `HR_ERROR_QUEUE_EMPTY`.
5. Nếu block: receiver lưu destination buffer và wait.

## Ordering

Queue data FIFO. Waiters ưu tiên effective priority; cùng priority FIFO.

## Direct handoff

Data được copy trước khi waiter READY. Điều này giữ semantics: waiter được wake vì operation đã hoàn thành, không phải chỉ vì "có thể thử lại".

## ISR

Send/receive ISR nonblocking và có `higher_priority_task_woken`.

## Invariants

- `count <= capacity`;
- head/tail trong range;
- sender wait khi không thể send;
- receiver wait khi không thể receive;
- wait buffer lifetime còn hợp lệ khi task block;
- item_size/capacity không đổi sau create.

## Chưa có

Variable-size message, zero-copy ownership, overwrite queue, queue set/select.
