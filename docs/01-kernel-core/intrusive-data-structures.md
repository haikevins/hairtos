# Intrusive data structures

## 1. Mục tiêu

Cho phép một task tham gia nhiều cấu trúc kernel mà không cấp phát node động.

## 2. Circular doubly linked list

`hr_list_t` chứa sentinel root và size. `hr_list_node_t` chứa `previous`, `next`, pointer tới list và `owner`.

```text
root <-> node A <-> node B <-> root
```

Node biết object sở hữu thông qua `owner`, vì vậy scheduler có thể chuyển từ ready node về TCB.

## 3. Node trong TCB

Mỗi task có node riêng cho:

- ready queue;
- wait list;
- timeout list;
- all-task list.

Một node không thể được insert vào hai list cùng lúc. API trả `HR_ERROR_INVALID_STATE` khi double insert/remove.

## 4. Ready set

`hr_ready_set_t` có một FIFO list cho mỗi priority và bitmap đánh dấu queue không rỗng. Priority nhỏ nhất có bit tương ứng và được chọn trước.

## 5. Wait list

Waiter được insert theo effective priority; cùng priority giữ FIFO. Queue, semaphore và mutex dùng cùng cấu trúc này.

## 6. Timeout list

Dùng hai ordered lists: current epoch và overflow epoch. Khi tick wrap, hai list đổi vai trò. Node cùng deadline giữ thứ tự FIFO.

## 7. Invariants

- `node->list` khớp list chứa node.
- Liên kết trước/sau đối xứng.
- `size` khớp số node thực.
- Ready bitmap khớp queue state.
- Timeout count bằng tổng hai list.

## 8. Kiểm thử

Host tests dùng ASan/UBSan và validate sau insert/remove/rotate/wrap. Đây là nền tảng cho mọi subsystem blocking.
