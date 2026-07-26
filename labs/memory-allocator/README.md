# Bài thực hành bộ cấp phát bộ nhớ

## 1. Mục đích

Memory allocator lab dùng để nghiên cứu cơ chế cấp phát, fragmentation và validation mà không làm `hairtos` phụ thuộc dynamic memory.

Kernel production vẫn tuân theo static-first:

- TCB do application cấp phát tĩnh;
- task stack là array tĩnh;
- queue storage do caller cung cấp;
- semaphore, mutex và timer là static objects;
- `haievent` dùng fixed-block event pool do application cung cấp;
- không có `malloc()` trong kernel hot path.

Lab cung cấp hai allocator độc lập: fixed-block pool và first-fit heap.

## 2. Phạm vi và trách nhiệm

Lab chịu trách nhiệm:

- quản lý arena do caller cung cấp;
- căn chỉnh payload theo `max_align_t`;
- fixed-size allocation từ free list;
- first-fit variable-size allocation;
- block splitting và adjacent coalescing;
- invalid pointer và double-free detection;
- structural validation;
- internal/external fragmentation statistics;
- demo host và target.

Lab không chịu trách nhiệm:

- tạo global heap;
- thay thế libc allocator;
- synchronization giữa nhiều task;
- allocation trong ISR;
- cấp phát TCB, queue, timer hoặc Active Object;
- real-time guarantee cho first-fit search và validation.

## 3. Cấu trúc thư mục

```text
labs/memory-allocator/
├── README.md
├── demo.c
├── include/
│   ├── hr_heap_lab.h
│   └── hr_pool_lab.h
├── src/
│   ├── hr_heap_lab.c
│   └── hr_pool_lab.c
└── tests/
    └── test_heap_lab.c
```

- `include/`: public API của lab;
- `src/`: implementation;
- `demo.c`: native host demonstration;
- `tests/`: allocator-specific host tests được tích hợp vào hairtos host suite.

## 4. Thành phần triển khai

### Pool khối cố định

`hr_pool_lab_t` chia arena thành các block có stride đã căn chỉnh.

Đặc tính:

- allocation lấy node đầu free list;
- free chèn block trở lại free list;
- allocation path O(1);
- validation của pointer/double free có thể duyệt metadata/free list;
- không có external fragmentation;
- có internal fragmentation do block stride lớn hơn requested block size.

### Heap first-fit

`hr_heap_lab_t` quản lý physical block chain:

```text
[header | payload][header | payload][header | payload]
```

Allocation:

1. căn requested size;
2. duyệt từ first block;
3. chọn free block đầu tiên đủ lớn;
4. split khi remainder đủ chứa header và minimum payload;
5. đánh dấu allocated và lưu requested size.

Free:

1. xác nhận pointer là đầu payload của một allocated block;
2. phát hiện outside/middle pointer hoặc double free;
3. đánh dấu free;
4. coalesce với next block;
5. coalesce với previous block nếu adjacent và free.

## 5. API công khai

### Status dùng chung

```c
typedef enum
{
    HR_HEAP_LAB_OK = 0,
    HR_HEAP_LAB_ERROR_INVALID_ARGUMENT,
    HR_HEAP_LAB_ERROR_NOT_INITIALIZED,
    HR_HEAP_LAB_ERROR_OUT_OF_MEMORY,
    HR_HEAP_LAB_ERROR_INVALID_POINTER,
    HR_HEAP_LAB_ERROR_DOUBLE_FREE,
    HR_HEAP_LAB_ERROR_CORRUPT
} hr_heap_lab_status_t;
```

### Heap first-fit

```c
hr_heap_lab_status_t hr_heap_lab_init(hr_heap_lab_t *heap,
                                      void *memory,
                                      size_t memory_bytes);
void *hr_heap_lab_alloc(hr_heap_lab_t *heap, size_t requested_bytes);
hr_heap_lab_status_t hr_heap_lab_free(hr_heap_lab_t *heap, void *pointer);
hr_heap_lab_status_t hr_heap_lab_get_stats(const hr_heap_lab_t *heap,
                                           hr_heap_lab_stats_t *stats);
bool hr_heap_lab_validate(const hr_heap_lab_t *heap);
size_t hr_heap_lab_alignment(void);
```

### Pool khối cố định

```c
hr_heap_lab_status_t hr_pool_lab_init(hr_pool_lab_t *pool,
                                      void *memory,
                                      size_t memory_bytes,
                                      size_t block_bytes,
                                      size_t block_count);
void *hr_pool_lab_alloc(hr_pool_lab_t *pool);
hr_heap_lab_status_t hr_pool_lab_free(hr_pool_lab_t *pool, void *pointer);
hr_heap_lab_status_t hr_pool_lab_get_stats(const hr_pool_lab_t *pool,
                                           hr_pool_lab_stats_t *stats);
bool hr_pool_lab_validate(const hr_pool_lab_t *pool);
```

## 6. Luồng hoạt động

### Cấp phát trên heap

```text
requested bytes
      |
      +--> align size
      |
      +--> first-fit physical chain search
      |
      +--> no block: failed_allocation_count++, NULL
      |
      +--> split nếu đủ remainder
      |
      +--> mark allocated và trả payload pointer
```

### Giải phóng heap và gộp khối

```text
pointer
   |
   +--> validate exact payload boundary
   |
   +--> reject invalid / double free
   |
   +--> mark free
   |
   +--> merge next
   |
   +--> merge previous
```

### Chỉ số phân mảnh

```text
internal fragmentation
= allocated payload capacity - requested bytes

external fragmentation
= total free payload - largest free block
```

Khi toàn bộ allocation được giải phóng và coalescing thành công, heap trở lại một free block và external fragmentation bằng 0.

## 7. Tích hợp build và quan hệ phụ thuộc

Module `allocator` trong `cmake/hairtos_modules.cmake` chứa:

```text
labs/memory-allocator/src/hr_heap_lab.c
labs/memory-allocator/src/hr_pool_lab.c
```

Target object nhận thêm include path:

```text
labs/memory-allocator/include
```

Host example 14 link cùng hai source và `demo.c`.

Dependency của allocator lab chỉ gồm C standard integer, size, alignment và caller-provided memory. Nó không include kernel internal headers và không gọi scheduler/critical-section API.

## 8. Biên dịch và kiểm tra

### Demo trên host

```bash
make ENVIRONMENT=host \
     EXAMPLE=14-memory-allocator-lab \
     run
```

Demo thực hiện:

- ba heap allocations;
- giải phóng block giữa để tạo fragmentation;
- giải phóng toàn bộ để kiểm tra coalescing;
- khởi tạo fixed-block pool;
- allocate hai block và validate pool.

### Demo trên target

```bash
make ENVIRONMENT=target \
     EXAMPLE=14-memory-allocator-lab \
     build

make ENVIRONMENT=target \
     EXAMPLE=14-memory-allocator-lab \
     run
```

Target dùng arena tĩnh trong application và in statistics qua USART1.

### Kiểm thử trên host

```bash
make host-tests
```

Các test allocator kiểm tra:

- alignment và minimum allocation;
- first-fit reuse;
- forward/backward coalescing;
- large allocation sau merge;
- internal/external fragmentation;
- invalid pointer và middle pointer;
- double free;
- exhaustion và failed counter;
- randomized deterministic allocation/free;
- pool alignment, reuse, exhaustion và diagnostics.

## 9. Bất biến và giới hạn

### Bất biến của heap

- arena begin/end và block chain phải nằm trong managed range;
- mỗi block header phải căn chỉnh đúng;
- block vật lý không overlap;
- chain phải kết thúc đúng tại arena boundary;
- allocated block phải giữ requested size hợp lệ;
- adjacent free blocks không được tồn tại sau coalescing hoàn chỉnh.

### Bất biến của pool

- block stride phải căn chỉnh;
- mọi free node phải trỏ tới block boundary trong arena;
- số free block không vượt capacity;
- cùng một block không được xuất hiện hai lần trong free list.

### Giới hạn

- Không thread-safe; caller phải serialize access.
- First-fit allocation và validation không có bounded constant time.
- Metadata nằm trong arena và có thể bị corruption nếu application ghi tràn payload.
- Không có canary cho từng allocation.
- Không có realloc/calloc.
- Không có ownership theo task.
- Không gọi từ ISR, SysTick, PendSV hoặc SVC.
- Đây là lab học tập, không phải production general-purpose allocator.

## 10. Mở rộng và tài liệu liên quan

Các hướng nghiên cứu phù hợp:

- segregated free lists;
- bitmap-based fixed pool;
- boundary tags;
- best-fit so với first-fit;
- per-task arena;
- lock wrapper bên ngoài allocator;
- allocation latency benchmark;
- corruption canary và poison pattern;
- deterministic fixed-size slab cho object cụ thể.

Mỗi thay đổi nên bổ sung:

1. structural invariant;
2. host tests với ASan/UBSan;
3. fragmentation/latency measurement;
4. tài liệu về worst-case behavior;
5. xác nhận kernel production vẫn không phụ thuộc allocator.

Tài liệu liên quan:

- `examples/14-memory-allocator-lab/README.md`;
- `docs/07-labs-and-examples/memory-allocator-lab.md`;
- `tests/host/test_main.c`;
- `labs/memory-allocator/tests/test_heap_lab.c`.
