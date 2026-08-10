# Memory allocator lab

## Mục tiêu

So sánh hai chiến lược caller-owned memory:

- fixed-block pool;
- first-fit heap.

Kernel v1 không dùng hai allocator này.

## Pool

Fixed stride free list. Không external fragmentation; có internal fragmentation theo block size.

## Heap

Block headers, alignment, first-fit, split và adjacent coalescing.

## Statistics

Theo dõi requested/allocated/free/largest block và fragmentation metrics.

## Safety checks

Host tests kiểm tra invalid pointer, double free, boundary, coalesce và randomized operation sequence.

## Vì sao không tích hợp kernel?

Static-first giúp TCB/queue/timer footprint deterministic và tránh allocator failure path trong scheduler. Lab tồn tại để học trade-off, không phải để vô tình làm kernel dynamic.

## V2

Giữ allocator ngoài core. Nếu v2 cần optional dynamic convenience API, nó nên là layer adapter rõ ràng trên caller-provided allocator, không làm core phụ thuộc heap.
