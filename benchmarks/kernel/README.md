# Hỗ trợ benchmark kernel

## 1. Mục đích

Thư mục `benchmarks/kernel/` cung cấp công cụ đo latency và chi phí thực thi của `hairtos` trên Cortex-M3. Module được link có chủ đích vào example `15-kernel-benchmark`, không được đưa vào firmware thông thường.

Mục tiêu:

- đo số cycle bằng DWT_CYCCNT;
- thu thập mẫu trong bộ nhớ tĩnh;
- tính min, max, mean và percentile;
- trừ measurement overhead;
- chuyển cycle sang nanosecond;
- tạo marker PB0 cho logic analyzer.

## 2. Phạm vi và trách nhiệm

Benchmark support chịu trách nhiệm cho cơ chế đo và thống kê, không quyết định workload kernel cần đo.

Workload cụ thể như SVC startup, PendSV round trip, queue wake-up, timer jitter hoặc `haievent` dispatch được định nghĩa trong:

```text
examples/15-kernel-benchmark/main.c
```

Module không:

- chèn instrumentation vào scheduler hot path bình thường;
- thay đổi queue, semaphore, mutex hoặc timer implementation;
- tự in UART trong lúc thu mẫu;
- cung cấp clock abstraction chung cho kernel;
- đảm bảo kết quả giống nhau giữa các compiler hoặc optimization level.

## 3. Cấu trúc thư mục

```text
benchmarks/kernel/
├── README.md
├── include/
│   └── hr_benchmark.h
└── src/
    ├── hr_benchmark_dwt.c
    ├── hr_benchmark_gpio.c
    └── hr_benchmark_stats.c
```

- `hr_benchmark.h`: public interface của benchmark module;
- `hr_benchmark_dwt.c`: khởi tạo và đọc DWT_CYCCNT;
- `hr_benchmark_gpio.c`: PB0 active-high timing marker;
- `hr_benchmark_stats.c`: sample buffer và thống kê.

## 4. Thành phần triển khai

### Bộ đếm chu kỳ DWT

`hr_benchmark_clock_init()`:

1. bật `DEMCR.TRCENA`;
2. kiểm tra DWT có cycle counter;
3. reset `DWT_CYCCNT`;
4. bật `DWT_CTRL.CYCCNTENA`;
5. thực hiện DSB/ISB;
6. chạy một chuỗi NOP để xác nhận counter tăng;
7. lưu core frequency.

### Thống kê dung lượng cố định

`hr_benchmark_stats_t` chứa tối đa:

```c
#define HR_BENCHMARK_MAX_SAMPLES 64U
```

Mỗi lần record cập nhật:

- sample array;
- total cycles dạng `uint64_t`;
- min;
- max;
- sample count.

Mean dùng phép chia `uint64_t / uint32_t` tự cài đặt để tránh kéo runtime division không mong muốn trong môi trường freestanding.

Percentile dùng nearest-rank sau khi copy và insertion-sort tối đa 64 mẫu.

### Tín hiệu đánh dấu GPIO

PB0 được cấu hình output push-pull 50 MHz:

```text
mark_begin() → PB0 high
mark_end()   → PB0 low
```

Marker cho phép so sánh DWT result với logic analyzer hoặc oscilloscope.

## 5. API công khai

### Thống kê

```c
void hr_benchmark_stats_reset(hr_benchmark_stats_t *stats);
bool hr_benchmark_stats_record(hr_benchmark_stats_t *stats, uint32_t cycles);
size_t hr_benchmark_stats_count(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_min(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_max(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_mean(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_percentile(const hr_benchmark_stats_t *stats,
                                       uint32_t percentile);
```

### Tiện ích chu kỳ

```c
uint32_t hr_benchmark_elapsed_cycles(uint32_t start_cycles,
                                     uint32_t end_cycles);
uint32_t hr_benchmark_adjust_cycles(uint32_t raw_cycles,
                                    uint32_t measurement_overhead_cycles);
uint32_t hr_benchmark_cycles_to_nanoseconds(uint32_t cycles,
                                            uint32_t core_clock_hz);
```

### Clock target và tín hiệu đánh dấu

```c
bool hr_benchmark_clock_init(uint32_t core_clock_hz);
uint32_t hr_benchmark_clock_now(void);
uint32_t hr_benchmark_clock_frequency_hz(void);

void hr_benchmark_gpio_init(void);
void hr_benchmark_gpio_mark_begin(void);
void hr_benchmark_gpio_mark_end(void);
```

## 6. Luồng hoạt động

```text
Khởi tạo DWT và PB0
        |
        +--> đo read overhead
        |
        +--> warm-up workload
        |
        +--> lặp N mẫu
                |
                +--> timestamp start
                +--> PB0 high nếu cần
                +--> workload
                +--> PB0 low
                +--> timestamp end
                +--> elapsed - overhead
                +--> record statistics
        |
        +--> dừng thu mẫu
        |
        +--> in UART report
```

UART report chỉ được phát sau khi thu mẫu để polling UART không làm sai latency đang đo.

Phép tính elapsed:

```c
end_cycles - start_cycles
```

sử dụng unsigned wrap semantics và hợp lệ nếu một phép đo không kéo dài quá toàn bộ chu kỳ wrap 32-bit.

## 7. Tích hợp build và quan hệ phụ thuộc

Module `benchmark` được khai báo trong `cmake/hairtos_modules.cmake` và chỉ được chọn bởi `15-kernel-benchmark`.

Mã nguồn module:

```text
benchmarks/kernel/src/hr_benchmark_stats.c
benchmarks/kernel/src/hr_benchmark_dwt.c
benchmarks/kernel/src/hr_benchmark_gpio.c
```

Include path bổ sung:

```text
benchmarks/kernel/include
```

Quan hệ phụ thuộc:

```text
benchmark stats → C standard integer/types only
DWT clock      → Cortex-M3 memory-mapped debug registers
GPIO marker    → public driver API hr_gpio.h
```

Example 15 được phép dùng một số kernel internal API có chủ đích để đo scheduler internals; quyền này chỉ áp dụng cho main của benchmark, không mở internal path cho các example khác.

## 8. Biên dịch và kiểm tra

Biên dịch firmware:

```bash
make EXAMPLE=15-kernel-benchmark build
```

Flash và chạy:

```bash
make EXAMPLE=15-kernel-benchmark run
```

Tạo disassembly:

```bash
make EXAMPLE=15-kernel-benchmark disasm
```

Chạy host tests cho statistics utilities:

```bash
make host-tests
```

Host test liên quan:

```text
tests/host/test_benchmark.c
```

Kết nối UART để đọc report và PB0/GND tới logic analyzer khi cần xác nhận marker.

## 9. Bất biến và giới hạn

- Tối đa 64 mẫu cho mỗi `hr_benchmark_stats_t`.
- Record thứ 65 trả `false`.
- Percentile lớn hơn 100 được clamp về 100.
- Empty statistics trả 0 cho min/max/mean/percentile.
- DWT_CYCCNT chỉ có trên target hỗ trợ cycle counter; `clock_init()` có thể trả `false`.
- Cycle-to-nanosecond là integer conversion và có rounding xuống.
- Kết quả phụ thuộc clock, compiler, toolchain, optimization, flash wait-state và interrupt load.
- Không nên in UART hoặc gọi API chậm bên trong vùng đo.
- PB0 phải không bị application khác sử dụng đồng thời.
- Benchmark code không phải production telemetry subsystem.

## 10. Mở rộng và tài liệu liên quan

Khi thêm metric:

1. xác định chính xác start/end boundary;
2. warm-up trước khi thu mẫu;
3. đo và trừ read overhead;
4. tránh UART trong vùng đo;
5. ghi rõ interrupts có được phép hay không;
6. xuất count, min, p50, mean, p95 và max;
7. thêm host test nếu bổ sung statistics logic;
8. cập nhật example 15 README và tài liệu benchmark.

Tài liệu liên quan:

- `examples/15-kernel-benchmark/README.md`;
- `docs/06-testing-and-quality/kernel-benchmark.md`;
- `docs/07-labs-and-examples/benchmark-lab.md` nếu có trong bản tài liệu sử dụng;
- `tests/host/test_benchmark.c`.
