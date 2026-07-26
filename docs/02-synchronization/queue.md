# Queue

## 1. Mục tiêu

Truyền item fixed-size theo FIFO giữa task hoặc từ ISR, hỗ trợ blocking và timeout.

## 2. Cấu trúc

Queue control block lưu storage pointer, item size, capacity, head, tail, count và hai wait lists: sender và receiver.

Application cung cấp storage:

```c
static hr_queue_t queue;
static message_t items[4];
hr_queue_create_static(&queue, items, sizeof(message_t), 4U);
```

## 3. Non-blocking path

- Send vào queue chưa đầy: copy item vào tail.
- Receive từ queue chưa rỗng: copy item tại head.
- `HR_NO_WAIT` trên full/empty trả lỗi tương ứng.

## 4. Direct handoff

Nếu receiver đang chờ, sender copy trực tiếp vào buffer của receiver rồi unblock nó. Nếu receiver vừa tạo slot trên queue đầy, item của blocked sender được copy ngay vào slot trước khi sender được đánh thức.

Cách này ngăn race “được đánh thức nhưng tài nguyên đã bị task khác chiếm”.

## 5. Blocking và timeout

Blocked task lưu item buffer trong TCB và được insert vào wait list theo effective priority. Finite timeout đồng thời insert timeout node. Completion path phải remove node còn lại và đặt `wait_result`.

## 6. ISR path

ISR API không block. Cờ higher-priority task woken cho biết cần pend PendSV.

## 7. Invariants

- `count <= capacity`.
- head/tail luôn trong range.
- sender chỉ chờ khi queue full; receiver chỉ chờ khi empty.
- task không nằm đồng thời trong send và receive wait list.

## 8. Giới hạn

Không hỗ trợ variable-size message, zero-copy buffer ownership hoặc queue overwrite mode.
