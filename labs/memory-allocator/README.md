# Bài thực hành bộ cấp phát bộ nhớ

## 1. Mục đích

`labs/memory-allocator/` là package học tập độc lập gồm fixed-block pool và first-fit heap. Kernel runtime không phụ thuộc allocator này; toàn bộ vùng nhớ do caller cung cấp tĩnh.

## 2. Phạm vi và trách nhiệm

Lab minh họa:

- alignment theo `max_align_t`;
- block header và payload;
- first-fit allocation;
- split và adjacent coalescing;
- invalid pointer/double free detection;
- internal và external fragmentation;
- validation trên host và target.

## 3. Cấu trúc thư mục

```text
labs/memory-allocator/
├── README.md
├── include/
│   ├── hr_heap_lab.h
│   └── hr_pool_lab.h
├── src/
│   ├── hr_heap_lab.c
│   └── hr_pool_lab.c
├── tests/test_heap_lab.c
└── demo.c
```

## 4. Thành phần triển khai

### Pool khối cố định

Pool chia arena thành các block cùng stride, quản lý free list và cung cấp allocation time xác định theo cấu trúc hiện tại.

### Heap first-fit

Heap duyệt block chain để tìm block đầu tiên đủ lớn, split phần dư khi hợp lệ và coalesce các block trống liền kề khi free.

## 5. API công khai

Các API chính:

```c
hr_heap_lab_init();
hr_heap_lab_alloc();
hr_heap_lab_free();
hr_heap_lab_get_stats();
hr_heap_lab_validate();

hr_pool_lab_init();
hr_pool_lab_alloc();
hr_pool_lab_free();
hr_pool_lab_get_stats();
hr_pool_lab_validate();
```

Chi tiết kiểu dữ liệu và status nằm trong hai public header của lab.

## 6. Luồng hoạt động

```text
caller-owned static arena
    |
    +--> init
    +--> allocate/free sequence
    +--> update block/free-list metadata
    +--> collect statistics
    +--> validate invariants
```

Allocator không gọi `malloc()` và không phụ thuộc scheduler.

## 7. Tích hợp build và dependency

Module `allocator` chỉ được link khi example 14 được chọn. Include path `labs/memory-allocator/include` không trở thành public include mặc định của kernel.

Host và target dùng cùng implementation; target khác nhau chỉ ảnh hưởng board output và toolchain, không ảnh hưởng thuật toán allocator.

## 8. Biên dịch và kiểm tra

Host demo:

```bash
make TARGET=bluepill_f103c8 \
     ENVIRONMENT=host \
     EXAMPLE=14-memory-allocator-lab \
     run
```

Target demo:

```bash
make TARGET=bluepill_f103c8 \
     ENVIRONMENT=target \
     EXAMPLE=14-memory-allocator-lab \
     run
```

Host tests:

```bash
make TARGET=bluepill_f103c8 host-tests
```

## 9. Bất biến và giới hạn

- Arena phải có alignment và kích thước hợp lệ.
- Mỗi block thuộc đúng một trạng thái allocated/free.
- Các block trống liền kề phải được coalesce sau free.
- Pool free list không được chứa duplicate node.
- Lab không thread-safe và không được gọi từ ISR.
- Đây không phải production allocator và không được kernel dùng ngầm.

## 10. Khả năng port và tài liệu liên quan

Allocator source không phụ thuộc MCU. Để chạy trên target mới, chỉ cần target đó cung cấp board initialization và output service mà example 14 sử dụng.

Tài liệu liên quan:

- [`../../docs/07-labs-and-examples/memory-allocator-lab.md`](../../docs/07-labs-and-examples/memory-allocator-lab.md);
- [`../../examples/14-memory-allocator-lab/README.md`](../../examples/14-memory-allocator-lab/README.md).

## Liên hệ audit và Version 2

Allocator lab tiếp tục là experiment riêng, không trở thành dependency của kernel. Static-first vẫn là nguyên tắc baseline của Version 2.

- [`../../docs/00-overview/design-principles.md`](../../docs/00-overview/design-principles.md)
- [`../../docs/09-version2/vision-and-goals.md`](../../docs/09-version2/vision-and-goals.md)
