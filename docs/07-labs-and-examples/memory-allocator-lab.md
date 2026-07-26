# Memory allocator lab

## 1. Mục tiêu

Học trade-off giữa fixed-block pool và first-fit heap mà không thay đổi static-first policy của kernel.

## 2. Fixed-block pool

O(1)-style free-list allocation, không external fragmentation, có internal fragmentation theo block stride. Phát hiện invalid pointer và double free.

## 3. First-fit heap

Arena chứa block headers; allocation căn chỉnh, split block đủ lớn; free coalesce hai phía. Stats gồm requested, allocated payload, free payload, largest block, internal/external fragmentation.

## 4. Host

```bash
make ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run
make host-tests
```

## 5. Target

```bash
make EXAMPLE=14-memory-allocator-lab run
```

Application dùng static arena và in stats qua UART.

## 6. Boundary

Không dùng allocator này để tạo task/queue/timer. Nó là lab và không được link vào example khác.

## 7. Giới hạn

Không thread-safe, không real-time guarantee cho first-fit search, không realloc/calloc.
