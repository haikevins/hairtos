# Mutex và priority inheritance

## Ownership

Mutex có owner. Chỉ owner được unlock.

Recursive mutex cho phép owner lock lại và tăng recursion count; non-recursive mutex không có behavior đó.

## Lock

Nếu free: current task trở owner và mutex được link vào owned-mutex list của task.

Nếu owner khác:

- no-wait -> busy/error;
- blocking -> waiter insert theo effective priority;
- nếu waiter cao hơn owner, recompute/propagate inheritance.

## Effective priority

```text
effective = min(base_priority,
                highest waiter priority trên mọi mutex đang giữ)
```

Nhớ rằng số nhỏ hơn = cao hơn.

Không restore đơn giản "về base" sau mỗi unlock; phải recompute vì task có thể còn giữ mutex khác với high-priority waiter.

## Chained inheritance

Nếu owner được boost nhưng chính owner đang block trên mutex khác, boost có thể propagate tới owner tiếp theo.

Implementation chặn traversal bằng `HR_CFG_MAX_TASKS` để tránh vô hạn nếu ownership graph corrupt/cyclic.

## Unlock

Recursive count >1: chỉ decrement.

Final unlock:

1. chọn waiter phù hợp;
2. chuyển owner trực tiếp;
3. update owned lists;
4. complete waiter;
5. recompute old owner;
6. recompute new owner nếu cần.

## Priority inversion scenario

```text
Low owns M
High waits M -> Low inherits High
Medium becomes READY
Low vẫn outrank Medium
Low unlocks
High owns M và chạy
```

## Chưa có

- deadlock detection;
- priority ceiling;
- robust mutex;
- owner-death recovery.

Deadlock diagnostics là candidate tốt cho v2, nhưng automatic prevention không cần là v2.0 requirement.
