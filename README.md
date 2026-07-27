# hairtos

`hairtos` là một RTOS mang tính giáo dục, ưu tiên cấp phát tĩnh, được tổ chức để có thể kiểm thử trên host và port sang nhiều MCU thông qua kiến trúc `TARGET` manifest. `haievent` là framework event-driven tùy chọn chạy phía trên public API của kernel.

Phiên bản hiện tại: **`1.0.0-rc1`**. Xem [`VERSION`](VERSION) và [`CHANGELOG.md`](CHANGELOG.md) để theo dõi phiên bản và thay đổi.

## 1. Mục tiêu dự án

Project hướng đến ba mục tiêu:

- giải thích các thành phần RTOS bằng source code nhỏ, có thể đọc, build và kiểm thử;
- cung cấp kernel static-first với task, scheduler, IPC, timer, diagnostics và fault retention;
- giữ ranh giới rõ giữa application, framework, kernel, architecture port, SoC, board và driver để việc thêm MCU mới không yêu cầu sửa kernel generic.

`hairtos` là codebase học tập có kiểm thử và quy trình port rõ ràng; nó chưa thay thế một RTOS production đã trưởng thành.

## 2. Tính năng hiện có

Kernel hỗ trợ:

- tạo task tĩnh và dựng initial stack frame qua architecture port;
- khởi chạy task đầu tiên bằng SVC trên port Cortex-M3 hiện tại;
- context switch bằng PendSV và PSP;
- fixed-priority preemptive scheduling;
- FIFO round-robin giữa các task cùng priority;
- tick, delay, periodic delay và timeout có xử lý wrap;
- queue blocking, direct handoff và API từ ISR;
- binary/counting semaphore và give từ ISR;
- recursive mutex và chained priority inheritance;
- suspend/resume cho task READY, RUNNING hoặc BLOCKED;
- software timer với timer-service task;
- retained panic/fault record, runtime statistics và kernel health check.

Framework `haievent` hỗ trợ:

- static event và dynamic event từ fixed-block pool;
- reference counting và ownership rules;
- flat state machine với ENTRY, EXIT và INIT;
- Active Object với queue riêng;
- time event dựa trên software timer;
- publish/subscribe nhiều subscriber.

Các phần nghiên cứu riêng gồm allocator lab, benchmark clock backend theo target và deterministic scheduler stress.

## 3. Hệ thống target

Build target được chọn bằng biến:

```bash
make TARGET=<target> EXAMPLE=<name> build
```

Target tham chiếu hiện tại:

| Thuộc tính | Giá trị |
| --- | --- |
| Target | `bluepill_f103c8` |
| MCU | STM32F103C8T6 |
| Board | Blue Pill |
| CPU | ARM Cortex-M3 |
| Clock danh định | 72 MHz |
| Debug | ST-Link V2, SWD, OpenOCD |
| UART tham chiếu | USART1, PA9/PA10, 115200 8-N-1 |

Xem target được hỗ trợ:

```bash
make list-targets
```

Mỗi target có một manifest trong `cmake/targets/<target>.cmake`. Manifest cung cấp CPU flags, compile definitions, include path, startup, linker script, architecture port, tick adapter, fault backend, driver implementation, benchmark clock và OpenOCD configuration.

## 4. Kiến trúc phụ thuộc

```text
Application / examples
        |
        +--> haievent public API
        |        |
        |        +--> hairtos public API
        |
        +------------> hairtos public API
                         |
                         +--> kernel internals
                                  |
                                  +--> architecture port
                                           |
                                           +--> SoC / board / drivers
```

Nguyên tắc:

- application bình thường không include `kernel/internal` hoặc `haievent/internal`;
- `haievent` chỉ phụ thuộc public API của `hairtos`;
- kernel C generic không phụ thuộc trực tiếp STM32F1 hoặc Blue Pill;
- exception entry, context switch, tick IRQ adapter và benchmark clock thuộc `arch/`;
- startup, clock và register map thuộc `soc/`;
- pin mapping, board service và linker script thuộc `boards/`;
- peripheral interface chung nằm trong `drivers/include`, còn encoding/instance cụ thể nằm trong `drivers/<soc>/include`;
- CMake cấp include scope theo từng object target để compiler bảo vệ ranh giới public/internal.

## 5. Cấu trúc repository

```text
hairtos/
├── arch/                         # Port CPU, exception, tick và benchmark clock
├── benchmarks/kernel/            # Thống kê benchmark độc lập phần cứng
├── boards/<board>/                # Board API, pin mapping và linker script
├── cmake/
│   ├── hairtos_examples.cmake     # Example → module/feature
│   ├── hairtos_modules.cmake      # Module → source
│   ├── hairtos_targets.cmake      # Phát hiện và kiểm tra target
│   └── targets/                   # Manifest cho từng hardware target
├── config/                        # Cấu hình kernel và haievent
├── docs/                          # Tài liệu kiến trúc, API, test và port
├── drivers/
│   ├── include/                   # Public peripheral API
│   └── <soc>/                     # Implementation và identifier theo SoC
├── examples/                      # Lộ trình 01–16
├── haievent/                      # Event framework
├── kernel/                        # Public API, internal model và implementation
├── labs/memory-allocator/         # Allocator lab độc lập
├── soc/<soc>/                     # Startup, clock, IRQ và register definitions
├── tests/                         # Host, mock, portability và stress tests
├── tools/                         # GDB/OpenOCD configuration
├── CMakeLists.txt
├── Makefile
├── VERSION
└── CHANGELOG.md
```

## 6. Bắt đầu nhanh

```bash
make help
make list-targets
make list-examples
```

Build image tích hợp mặc định:

```bash
make build
```

Tương đương:

```bash
make TARGET=bluepill_f103c8 \
     ENVIRONMENT=target \
     EXAMPLE=16-diagnostics-stress-stabilization \
     build
```

Flash target tham chiếu:

```bash
make TARGET=bluepill_f103c8 \
     EXAMPLE=16-diagnostics-stress-stabilization \
     run
```

Chạy host example:

```bash
make TARGET=bluepill_f103c8 \
     ENVIRONMENT=host \
     EXAMPLE=02-kernel-data-structures-host \
     run
```

Chạy toàn bộ host tests:

```bash
make TARGET=bluepill_f103c8 host-tests
```

## 7. Giao diện Make

```bash
make TARGET=<target> EXAMPLE=<name> build [ENVIRONMENT=host|target]
make TARGET=<target> EXAMPLE=<name> run   [ENVIRONMENT=host|target]
make TARGET=<target> EXAMPLE=<name> check [ENVIRONMENT=host|target]
make TARGET=<target> EXAMPLE=<name> clean [ENVIRONMENT=host|target]
```

| Action | Host | Target |
| --- | --- | --- |
| `build` | Compile executable native | Build `hairtos.elf`, `.bin`, `.hex` và size |
| `run` | Chạy executable với sanitizer | Flash, verify và reset qua OpenOCD |
| `check` | Chạy host tests rồi build example | Chạy host tests rồi cross-build firmware |
| `clean` | Xóa build của example | Xóa build theo target/example |

Lệnh bổ sung:

```bash
make list-targets
make list-examples
make TARGET=<target> EXAMPLE=<name> intellisense
make TARGET=<target> EXAMPLE=<name> size
make TARGET=<target> EXAMPLE=<name> disasm
make TARGET=<target> EXAMPLE=<name> gdb
make TARGET=<target> erase
make TARGET=<target> debug-server
make clean-all
```

CMake là nguồn dữ liệu duy nhất cho target, example, module, source và compile definition. Makefile chỉ là command wrapper.

## 8. Build trực tiếp bằng CMake

Target với GNU Arm:

```bash
cmake -S . -B build/manual-target \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm-none-eabi-gcc.cmake \
  -DHAIRTOS_ENVIRONMENT=target \
  -DHAIRTOS_TARGET=bluepill_f103c8 \
  -DHAIRTOS_EXAMPLE=09-queue-blocking-ipc

cmake --build build/manual-target
```

Host allocator lab:

```bash
cmake -S . -B build/manual-host \
  -G Ninja \
  -DHAIRTOS_ENVIRONMENT=host \
  -DHAIRTOS_TARGET=bluepill_f103c8 \
  -DHAIRTOS_EXAMPLE=14-memory-allocator-lab

cmake --build build/manual-host
```

## 9. Thiết kế phục vụ port

Các phần được giữ generic khi thêm MCU mới:

```text
kernel/
haievent/
benchmarks/kernel/src/hr_benchmark_stats.c
labs/memory-allocator/
phần lớn tests/host/
```

Các phần target mới phải cung cấp:

```text
arch/<architecture>/
soc/<soc>/
boards/<board>/
drivers/<soc>/
tools/openocd/<target>.cfg
cmake/targets/<target>.cmake
```

`hr_port_config.h` mô tả capability của port, gồm minimum stack, alignment, FPU context và MPU support. Điều này ngăn cấu hình kernel giả định cứng Cortex-M3.

GPIO pin và UART instance là opaque identifier. Public API không yêu cầu application biết GPIO port number hoặc peripheral clock. Board layer cung cấp LED, UART, benchmark marker, memory footprint và tên CPU.

## 10. Thêm target MCU mới

1. Sao chép `cmake/targets/target_template.cmake.example`.
2. Tạo architecture port hoặc dùng lại port hiện có.
3. Thêm SoC startup, clock, IRQ và register definitions.
4. Thêm board API, linker script và pin mapping.
5. Thêm driver implementation theo SoC.
6. Thêm OpenOCD configuration.
7. Điền target manifest và chạy `make list-targets`.
8. Cross-build các example từ đơn giản đến tích hợp.
9. Chạy trên hardware và kiểm tra startup, tick, context switch, fault, UART và benchmark clock.

Hướng dẫn chi tiết: [`docs/04-platform/porting-new-target.md`](docs/04-platform/porting-new-target.md).

## 11. Examples

Phân loại đầy đủ nằm trong [`examples/README.md`](examples/README.md).

```bash
# Host-only intrusive structures
make TARGET=bluepill_f103c8 EXAMPLE=02-kernel-data-structures-host run

# Target scheduler và round-robin
make TARGET=bluepill_f103c8 EXAMPLE=08-preemption-round-robin run

# Allocator lab trên host
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run

# Allocator lab trên target
make TARGET=bluepill_f103c8 ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run

# Kernel benchmark
make TARGET=bluepill_f103c8 EXAMPLE=15-kernel-benchmark run

# Diagnostics stress trên host
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run
```

## 12. Kiểm thử và chất lượng

```bash
make TARGET=bluepill_f103c8 host-tests
```

Bộ test bao phủ intrusive list, scheduler, timeout, task, queue, semaphore, mutex, timer, `haievent`, allocator, benchmark statistics, diagnostics và scheduler stress.

Build target chỉ xác nhận compile/link. Các thuộc tính thời gian thực, interrupt priority, reset retention, GPIO marker và clock phải được kiểm chứng trên MCU vật lý.

## 13. VS Code và IntelliSense

Tạo compile database cho target/example đang làm:

```bash
make TARGET=bluepill_f103c8 \
     EXAMPLE=16-diagnostics-stress-stabilization \
     intellisense
```

Sau đó chọn cấu hình `hairtos - Active CMake Build`. Xem [`.vscode/README.md`](.vscode/README.md).

## 14. Tài liệu

Bắt đầu tại [`docs/README.md`](docs/README.md). Những tài liệu quan trọng cho portability:

- [`docs/00-overview/dependency-rules.md`](docs/00-overview/dependency-rules.md);
- [`docs/04-platform/cortex-m3-port.md`](docs/04-platform/cortex-m3-port.md);
- [`docs/04-platform/startup-and-linker.md`](docs/04-platform/startup-and-linker.md);
- [`docs/04-platform/drivers.md`](docs/04-platform/drivers.md);
- [`docs/04-platform/porting-new-target.md`](docs/04-platform/porting-new-target.md).

## 15. Giới hạn và giấy phép

- Hiện repository mới có một target hoàn chỉnh: `bluepill_f103c8`.
- Port Cortex-M3 đã chạy trên thiết kế STM32F103; target mới vẫn cần hardware validation.
- Cortex-M4F cần port lưu FPU context trước khi bật `HR_CFG_USE_FPU`.
- Cortex-M0/M0+ hoặc kiến trúc khác cần assembly context switch và exception backend riêng.
- Driver hiện là polling/minimal; chưa có DMA hoặc generic asynchronous peripheral framework.

Project được phát hành theo giấy phép trong [`LICENSE`](LICENSE).
