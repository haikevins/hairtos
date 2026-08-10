# Semaphore

## Counting model

Semaphore có:

```text
count
max_count
waiters
```

Binary semaphore là counting semaphore max=1.

## Take

Nếu count > 0, decrement ngay.

Nếu 0:

- no-wait -> `HR_ERROR_SEMAPHORE_EMPTY`;
- finite/forever -> block trên waiter list.

## Give

Nếu waiter tồn tại, token được handoff trực tiếp cho waiter priority cao nhất; không cần increment count trung gian.

Nếu không có waiter, increment tới max. Give khi full trả `HR_ERROR_SEMAPHORE_FULL`.

## ISR give

`hr_semaphore_give_from_isr()` dùng cùng handoff model nhưng không block.

## Semaphore khác mutex

Semaphore không có owner. Task A có thể give token task B take. Vì vậy không dùng semaphore thay mutex khi cần ownership + priority inheritance.

## Invariants

- `count <= max_count`;
- max > 0;
- waiter ordering đúng;
- token không vừa increment vừa handoff cùng lần give.

## Chưa có

Take-from-ISR, delete/reset semaphore.
