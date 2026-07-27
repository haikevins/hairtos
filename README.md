# hairtos

`hairtos` là một hệ điều hành thời gian thực mang tính giáo dục, ưu tiên cấp phát tĩnh, được viết cho ARM Cortex-M. Target tham chiếu hiện tại là STM32F103C8T6 Blue Pill; `haievent` là framework event-driven tùy chọn chạy phía trên public API của kernel.

Phiên bản hiện tại: **`1.0.0-rc1`**. Xem [`VERSION`](VERSION) và [`CHANGELOG.md`](CHANGELOG.md) để theo dõi phiên bản và thay đổi.

## 1. Mục tiêu dự án

Project được xây dựng với ba mục tiêu chính:

- giải thích từng thành phần cốt lõi của RTOS bằng source code nhỏ, có thể đọc và kiểm thử;
- cung cấp một kernel Cortex-M3 chạy được với task, scheduling, IPC, timer và diagnostics;
- giữ ranh giới rõ giữa application, `haievent`, public kernel API, kernel internal, port, SoC, board và driver.

`hairtos` không nhằm thay thế ngay một RTOS production đã trưởng thành. Đây là codebase học tập có kiểm thử, có build host/target và có lộ trình mở rộng rõ ràng.

## 2. Tính năng hiện có

Kernel hiện hỗ trợ:

- tạo task tĩnh và khởi tạo initial Cortex-M3 stack frame;
- khởi chạy task đầu tiên bằng SVC;
- context switch bằng PendSV và PSP;
- bộ lập lịch chiếm quyền với ưu tiên cố định;
- FIFO round-robin giữa các task cùng priority;
- SysTick 1 kHz, delay, periodic delay và timeout có xử lý tick wrap;
- queue blocking, direct handoff và API từ ISR;
- counting/binary semaphore và give từ ISR;
- mutex ownership, recursive mutex và chained priority inheritance;
- suspend/resume cho task READY, RUNNING và BLOCKED;
- software timer chạy callback trong timer-service task;
- retained panic/fault record, runtime statistics và kernel health check.

Framework `haievent` hiện hỗ trợ:

- static event và dynamic event từ fixed-block pool;
- reference counting và ownership rules;
- flat state machine với ENTRY, EXIT và INIT;
- Active Object với event queue riêng;
- time event dựa trên software timer;
- publish/subscribe nhiều subscriber.

Các phần nghiên cứu riêng gồm allocator lab, DWT benchmark và deterministic scheduler stress.

## 3. Nền tảng và công cụ

### Target tham chiếu

| Thành phần | Cấu hình |
|---|---|
| MCU | STM32F103C8T6 |
| Board | Blue Pill |
| CPU | ARM Cortex-M3 |
| Clock | 72 MHz từ HSE 8 MHz và PLL x9 |
| Flash/RAM model | 64 KiB Flash / 20 KiB SRAM |
| Debug probe | ST-Link V2 |
| Debug protocol | SWD |
| UART | USART1, PA9 TX, PA10 RX, 115200 8-N-1 |

### Công cụ build

Target build hỗ trợ:

- GNU Arm Embedded Toolchain: `arm-none-eabi-gcc`;
- Clang/LLD cross-build;
- CMake 3.20 trở lên;
- Ninja;
- OpenOCD;
- `arm-none-eabi-gdb` hoặc `gdb-multiarch`.

Host build dùng GCC hoặc Clang native cùng AddressSanitizer và UndefinedBehaviorSanitizer.

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
                                  +--> Cortex-M3 port
                                           |
                                           +--> SoC / board / drivers
```

Nguyên tắc chính:

- application không include `kernel/internal` hoặc `haievent/internal`;
- `haievent` chỉ phụ thuộc public API của `hairtos`;
- code kernel thuần C không phụ thuộc trực tiếp board;
- assembly context switch và fault entry nằm trong `arch/arm/cortex-m3`;
- startup, clock và register map nằm trong `soc/stm32f1`;
- pin mapping và linker script thuộc board;
- allocator lab và benchmark không trở thành dependency ẩn của kernel bình thường.

CMake áp dụng include scope riêng cho từng object target để compiler thực sự bảo vệ ranh giới public/internal.

## 5. Cấu trúc repository

```text
hairtos/
├── arch/                  # Cortex-M3 port, context switch và fault entry
├── benchmarks/kernel/     # DWT cycle counter, statistics và GPIO marker
├── boards/                # Board API, pin mapping và linker script
├── cmake/                 # Example/module mapping và toolchain files
├── config/                # Cấu hình hairtos và haievent
├── docs/                  # Tài liệu kiến trúc, API, testing và labs
├── drivers/               # Driver API chung và implementation theo SoC
├── examples/              # Lộ trình 01–16, mỗi bài có main.c và README.md
├── haievent/              # Event framework public/internal/source
├── kernel/                # Kernel public API, internal model và implementation
├── labs/memory-allocator/ # Fixed-block pool và first-fit heap lab
├── soc/                   # STM32F1 startup, clock, IRQ và register definitions
├── tests/                 # Host tests, mocks, portability và stress
├── tools/                 # GDB và OpenOCD configuration
├── CMakeLists.txt
├── Makefile
├── VERSION
└── CHANGELOG.md
```

Chi tiết từng thư mục được mô tả trong [`docs/00-overview/project-layout.md`](docs/00-overview/project-layout.md).

## 6. Bắt đầu nhanh

### Xem lệnh và danh sách example

```bash
make help
make list-targets
make list-examples
```

### Build image tích hợp mặc định

```bash
make build
```

Lệnh mặc định tương đương:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=16-diagnostics-stress-stabilization \
     ENVIRONMENT=target \
     build
```

### Flash target

```bash
make TARGET=bluepill_f103c8 EXAMPLE=16-diagnostics-stress-stabilization run
```

`run` trong target mode thực hiện build, OpenOCD program, verify và reset.

### Chạy host example

```bash
make TARGET=bluepill_f103c8 EXAMPLE=02-kernel-data-structures-host run
```

### Chạy toàn bộ host tests

```bash
make host-tests
```

## 7. Giao diện Make thống nhất

Host và target dùng cùng bố cục:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=<name> build [ENVIRONMENT=host|target]
make TARGET=bluepill_f103c8 EXAMPLE=<name> run   [ENVIRONMENT=host|target]
make TARGET=bluepill_f103c8 EXAMPLE=<name> check [ENVIRONMENT=host|target]
make TARGET=bluepill_f103c8 EXAMPLE=<name> clean [ENVIRONMENT=host|target]
```

Ý nghĩa:

| Action | Host | Target |
|---|---|---|
| `build` | Compile executable native | Build `hairtos.elf`, `.bin`, `.hex` và size |
| `run` | Chạy executable với sanitizer | Flash, verify và reset board |
| `check` | Chạy host tests rồi build example | Chạy host tests rồi cross-build firmware |
| `clean` | Xóa build của example | Xóa build của example |

Các lệnh bổ sung:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=<name> intellisense [ENVIRONMENT=host|target]
make TARGET=bluepill_f103c8 EXAMPLE=<name> size
make TARGET=bluepill_f103c8 EXAMPLE=<name> disasm
make TARGET=bluepill_f103c8 EXAMPLE=<name> gdb
make erase
make debug-server
make clean-all
```

CMake là nguồn duy nhất quản lý example, module, source và compile definition. Makefile chỉ là command wrapper ổn định cho người dùng.

## 8. Ví dụ trên host và target

Phân loại đầy đủ nằm trong [`examples/README.md`](examples/README.md). Một số ví dụ thường dùng:

```bash
# Host-only intrusive structures
make TARGET=bluepill_f103c8 EXAMPLE=02-kernel-data-structures-host run

# Target scheduler và round-robin
make TARGET=bluepill_f103c8 EXAMPLE=08-preemption-round-robin run

# Allocator lab trên host
make ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run

# Allocator lab trên STM32
make ENVIRONMENT=target EXAMPLE=14-memory-allocator-lab run

# Kernel benchmark trên STM32
make TARGET=bluepill_f103c8 EXAMPLE=15-kernel-benchmark run

# Diagnostics stress trên host
make ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run
```

## 9. Build trực tiếp bằng CMake

Ví dụ target Clang/LLD:

```bash
cmake -S . -B build/manual-target \
  -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm-none-eabi-clang.cmake \
  -DHAIRTOS_ENVIRONMENT=target \
  -DHAIRTOS_EXAMPLE=09-queue-blocking-ipc

cmake --build build/manual-target
```

Ví dụ host allocator lab:

```bash
cmake -S . -B build/manual-host \
  -G Ninja \
  -DHAIRTOS_ENVIRONMENT=host \
  -DHAIRTOS_EXAMPLE=14-memory-allocator-lab

cmake --build build/manual-host
```

Danh sách source và feature của từng example nằm trong:

```text
cmake/hairtos_examples.cmake
cmake/hairtos_modules.cmake
```

Không nên tạo danh sách source thứ hai trong Makefile.

## 10. Kiểm thử và chất lượng

Host suite bao phủ:

- intrusive list, ready queue, wait list và timeout list;
- TCB, initial stack và task-state transition;
- scheduler, delay, preemption và time slice;
- queue, semaphore, mutex và priority inheritance;
- suspend/resume và software timer;
- `haievent` ownership, state machine và publish/subscribe;
- allocator statistics, split/coalesce và invalid free;
- benchmark statistics;
- diagnostics và deterministic stress invariants.

Chạy:

```bash
make host-tests
```

`check` kết hợp host suite với build của example được chọn:

```bash
make TOOLCHAIN=clang \
     EXAMPLE=16-diagnostics-stress-stabilization \
     check
```

Cross-build thành công không thay thế runtime validation trên phần cứng. Timing, fault retention qua reset, UART output và stress dài hạn vẫn phải được xác nhận trên Blue Pill thật.

## 11. VS Code và IntelliSense

Project cung cấp cấu hình trong [`.vscode/README.md`](.vscode/README.md).

Cách nhanh nhất để duyệt toàn source:

1. mở đúng thư mục gốc chứa `Makefile`;
2. chọn cấu hình **`hairtos - All Sources`**;
3. chạy `C/C++: Reset IntelliSense Database`;
4. reload VS Code.

Để dùng compile flags chính xác của một example:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=16-diagnostics-stress-stabilization \
     ENVIRONMENT=target \
     intellisense
```

Sau đó chọn **`hairtos - Active CMake Build`**.

## 12. Tài liệu

Điểm bắt đầu đề xuất:

- [`docs/README.md`](docs/README.md): chỉ mục tài liệu;
- [`docs/00-overview/architecture.md`](docs/00-overview/architecture.md): kiến trúc tổng thể;
- [`docs/01-kernel-core/README.md`](docs/01-kernel-core/README.md): task, scheduler và context switch;
- [`docs/02-synchronization/README.md`](docs/02-synchronization/README.md): queue, semaphore, mutex và timer;
- [`docs/03-haievent/README.md`](docs/03-haievent/README.md): framework sự kiện;
- [`docs/05-api-reference/README.md`](docs/05-api-reference/README.md): public API;
- [`docs/06-testing-and-quality/testing-guide.md`](docs/06-testing-and-quality/testing-guide.md): quy trình kiểm thử;
- [`examples/README.md`](examples/README.md): lộ trình example.

## 13. Giới hạn hiện tại

- Chỉ có port context-switch hoàn chỉnh cho ARM Cortex-M3.
- Target tham chiếu duy nhất là STM32F103C8T6 Blue Pill.
- Driver hiện là polling-oriented và chưa có DMA abstraction.
- Kernel không dùng dynamic allocation; allocator chỉ là lab độc lập.
- State machine của `haievent` là flat state machine, chưa phải hierarchical state machine.
- Chưa có SMP, MPU process isolation, tickless idle hoặc userspace/kernelspace separation.
- Các con số benchmark phải được đo lại trên phần cứng cụ thể.

## 14. Đóng góp và giấy phép

Khi sửa code:

- giữ quy tắc dependency trong `docs/00-overview/dependency-rules.md`;
- không đưa internal include vào application bình thường;
- cập nhật `cmake/hairtos_examples.cmake` hoặc `cmake/hairtos_modules.cmake` khi thêm module/example;
- thêm host test cho logic độc lập phần cứng;
- build ít nhất một target example liên quan;
- cập nhật `CHANGELOG.md` cho thay đổi đáng chú ý.

Project được phát hành theo giấy phép MIT. Xem [`LICENSE`](LICENSE).


## Port sang MCU mới

Mỗi phần cứng được mô tả bằng một target manifest trong `cmake/targets/`.
CMake và Makefile không chứa source STM32F1 hoặc Blue Pill cố định. Xem
[`docs/04-platform/porting-new-target.md`](docs/04-platform/porting-new-target.md)
để biết contract của architecture, SoC, board, driver và linker script.
