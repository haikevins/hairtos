# Hỗ trợ benchmark kernel

## 1. Mục đích

`benchmarks/kernel/` cung cấp phần thống kê benchmark độc lập phần cứng. Clock đo chu kỳ và GPIO marker được cung cấp bởi architecture/board target thay vì bị hard-code vào benchmark generic.

## 2. Phạm vi và trách nhiệm

Subsystem này chịu trách nhiệm:

- lưu sample trong mảng tĩnh;
- tính min, max, mean và percentile;
- điều chỉnh measurement overhead;
- đổi cycle sang nanosecond khi biết clock;
- định nghĩa contract cho benchmark clock backend.

Subsystem không lựa chọn DWT, GPIO pin, CPU name hoặc memory map.

## 3. Cấu trúc thư mục

```text
benchmarks/kernel/
├── README.md
├── include/hr_benchmark.h
└── src/hr_benchmark_stats.c
```

Backend target tham chiếu:

```text
arch/arm/cortex-m3/hr_benchmark_clock_dwt.c
```

Board marker và footprint API:

```text
boards/<board>/include/board.h
boards/<board>/board.c
```

## 4. Thành phần triển khai

### Thống kê

`hr_benchmark_stats_t` chứa tối đa `HR_BENCHMARK_MAX_SAMPLES` sample và các aggregate cần thiết. Không có dynamic allocation.

### Benchmark clock

Public contract:

```c
bool hr_benchmark_clock_init(uint32_t core_clock_hz);
uint32_t hr_benchmark_clock_now(void);
uint32_t hr_benchmark_clock_frequency_hz(void);
const char *hr_benchmark_clock_name(void);
```

Cortex-M3 target hiện dùng DWT CYCCNT. Target khác có thể dùng DWT, machine timer, GPT hoặc timer ngoại vi, miễn giữ contract.

### Board marker và footprint

Example 15 gọi:

```c
board_benchmark_marker_init();
board_benchmark_marker_begin();
board_benchmark_marker_end();
board_benchmark_marker_description();
board_get_flash_image_bytes();
board_get_static_ram_bytes();
board_get_cpu_name();
```

Pin marker, memory symbols và CPU description thuộc board implementation.

## 5. API công khai

```c
void hr_benchmark_stats_reset(hr_benchmark_stats_t *stats);
bool hr_benchmark_stats_record(hr_benchmark_stats_t *stats,
                               uint32_t cycles);
size_t hr_benchmark_stats_count(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_min(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_max(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_mean(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_percentile(const hr_benchmark_stats_t *stats,
                                       uint32_t percentile);
uint32_t hr_benchmark_elapsed_cycles(uint32_t start_cycles,
                                     uint32_t end_cycles);
uint32_t hr_benchmark_adjust_cycles(uint32_t raw_cycles,
                                    uint32_t measurement_overhead_cycles);
uint32_t hr_benchmark_cycles_to_nanoseconds(uint32_t cycles,
                                            uint32_t core_clock_hz);
```

## 6. Luồng hoạt động

```text
example 15
    |
    +--> board_get_cpu_name()
    +--> hr_benchmark_clock_init()
    +--> board_benchmark_marker_init()
    |
    +--> marker_begin()
    +--> start = clock_now()
    +--> operation under test
    +--> end = clock_now()
    +--> marker_end()
    |
    +--> adjust overhead
    +--> record sample
    +--> report statistics after collection
```

UART output được trì hoãn khỏi vùng đo để giảm perturbation.

## 7. Tích hợp build và dependency

Module `benchmark` trong `cmake/hairtos_modules.cmake` gồm:

```text
benchmarks/kernel/src/hr_benchmark_stats.c
${HAIRTOS_TARGET_BENCHMARK_CLOCK_C}
```

Target manifest chọn backend clock. Example 15 lấy board hooks qua public `board.h`; benchmark generic không include SoC register header.

## 8. Biên dịch và kiểm tra

```bash
make TARGET=bluepill_f103c8 EXAMPLE=15-kernel-benchmark build
make TARGET=bluepill_f103c8 EXAMPLE=15-kernel-benchmark run
make TARGET=bluepill_f103c8 host-tests
```

Host tests kiểm tra statistics, percentile, wrap-safe elapsed cycles, overhead adjustment và conversion.

## 9. Bất biến và giới hạn

- Sample count không vượt `HR_BENCHMARK_MAX_SAMPLES`.
- Percentile được tính trên bản sao/sắp xếp phù hợp với implementation hiện tại.
- Clock backend phải là monotonic modulo 32-bit trong thời gian một sample.
- Kết quả phụ thuộc target, clock, compiler, optimization, interrupt load và debugger.
- Build PASS không chứng minh độ chính xác của clock backend trên hardware.
- Example benchmark được phép truy cập internal scheduler API có chủ đích; đây không phải mẫu application thông thường.

## 10. Thêm target mới

Target mới cần cung cấp:

```cmake
HAIRTOS_TARGET_BENCHMARK_CLOCK_C
```

Board mới cần cung cấp CPU name, marker hooks và linker-derived footprint. Nếu target không có marker hoặc cycle counter, backend có thể trả trạng thái unavailable và example phải báo rõ thay vì dùng giá trị giả.

Tài liệu liên quan:

- [`../../docs/06-testing-and-quality/kernel-benchmark.md`](../../docs/06-testing-and-quality/kernel-benchmark.md);
- [`../../docs/04-platform/porting-new-target.md`](../../docs/04-platform/porting-new-target.md).
