# hairtos

`hairtos` là một RTOS static-first dành cho vi điều khiển nhỏ, được xây dựng với ba lớp chính:

```text
Preemptive RTOS Kernel
        +
Event-Driven Framework (`haievent`)
        +
State Machine / Active Object
```

Mục tiêu của project là làm rõ **bản chất của một RTOS và một framework event-driven** bằng source code có thể đọc, kiểm thử trên host và port sang nhiều MCU. Phiên bản hiện tại là **`1.0.0-rc1`**; toàn bộ tài liệu trong `docs/00`–`08` mô tả implementation v1 hiện hành, còn `docs/09-version2/` chỉ mô tả kế hoạch tương lai và **không được hiểu là tính năng đã triển khai**.

## 1. Kiến trúc tổng thể

```text
Application / examples
        |
        +------------------------+
        |                        |
        v                        v
  haievent public API       hairtos public API
        |                        |
        +-----------+------------+
                    v
              Kernel internals
                    |
                    v
             Architecture port
                    |
             +------+------+ 
             |             |
             v             v
            SoC          Board
             \             /
              +-----+-----+
                    v
                 Drivers
```

`hairtos` dùng fixed-priority preemptive scheduling. `haievent` không thay scheduler: mỗi Active Object hiện sở hữu một RTOS task, một event queue và một flat state machine; kernel vẫn quyết định task/AO nào được CPU chạy dựa trên priority.

## 2. Những gì v1 đã có

### Kernel

- task tạo tĩnh bằng caller-owned TCB storage và stack;
- first-task startup qua architecture port;
- Cortex-M3 hiện dùng SVC để vào task đầu tiên và PendSV để context switch;
- fixed-priority scheduler, priority số nhỏ hơn là cao hơn;
- preemption và FIFO round-robin giữa task cùng priority;
- tick, blocking delay, `delay_until()` và timeout có xử lý wrap-around;
- queue FIFO với blocking send/receive, timeout, direct handoff và ISR API;
- binary/counting semaphore và `give_from_isr`;
- non-recursive/recursive mutex với chained priority inheritance;
- suspend/resume task READY, RUNNING và BLOCKED;
- software timer với callback chạy trong timer-service task;
- stack fill/guard, runtime counters, kernel invariant check và retained panic/fault record.

### `haievent`

- static event và dynamic event từ fixed-block pool;
- event reference counting;
- flat state machine với `ENTRY`, `EXIT`, `INIT`;
- Active Object = task + queue + state machine;
- time event dựa trên software timer;
- publish/subscribe;
- post từ task và post từ ISR.

### Tooling và portability

- Makefile là command wrapper, CMake là source-of-truth cho source/module/example/target;
- `TARGET=<name>` chọn target manifest;
- architecture, SoC, board và driver được tách riêng;
- GPIO pin/UART instance là target-defined opaque identifiers;
- kernel tick IRQ không nằm trong kernel generic;
- benchmark clock/marker do architecture/board cung cấp;
- host unit tests, deterministic scheduler stress, allocator lab và target examples 01–16.

## 3. Trạng thái thực tế của implementation

| Khối | Trạng thái v1 |
|---|---|
| Static task + initial stack | Hoàn thành |
| Cooperative context switch | Hoàn thành |
| Fixed-priority preemption | Hoàn thành |
| Equal-priority round-robin | Hoàn thành |
| Delay/timeout | Hoàn thành |
| Queue | Hoàn thành |
| Semaphore | Hoàn thành |
| Mutex + priority inheritance | Hoàn thành |
| Suspend/resume | Hoàn thành |
| Software timer | Hoàn thành |
| Diagnostics/fault retention | Hoàn thành |
| Dynamic kernel allocation | Không có theo chủ đích |
| SMP | Không hỗ trợ |
| FPU context | Chưa hỗ trợ |
| MPU isolation | Chưa hỗ trợ |
| HSM | Chưa hỗ trợ |
| Deferred event | Chưa hỗ trợ |
| Shared-executor Active Object | Chưa hỗ trợ |
| Tickless idle | Chưa hỗ trợ |

Ma trận đầy đủ: [`docs/00-overview/capability-matrix.md`](docs/00-overview/capability-matrix.md).

## 4. Target hiện tại

Target hoàn chỉnh hiện có:

```text
bluepill_f103c8
```

Binding:

```text
MCU:        STM32F103C8T6
CPU:        ARM Cortex-M3
Board:      Blue Pill
Nominal:    72 MHz
UART:       USART1 PA9/PA10, 115200 8-N-1
Debug:      ST-Link / SWD / OpenOCD
LED:        PC13 active-low
Benchmark:  DWT CYCCNT + board marker
```

Xem target:

```bash
make list-targets
```

Việc có abstraction portability **không tự chứng minh** target thứ hai đã chạy. v1 mới có một target hoàn chỉnh; đây là một trong các mục tiêu quan trọng của Version 2.

## 5. Cấu trúc repository

```text
hairtos/
├── arch/                         # CPU/ISA-specific port
├── benchmarks/kernel/            # Benchmark statistics generic
├── boards/                       # Board services + linker
├── cmake/
│   ├── hairtos_examples.cmake
│   ├── hairtos_modules.cmake
│   ├── hairtos_targets.cmake
│   └── targets/
├── config/                       # Kernel + haievent config
├── docs/                         # Tài liệu v1 + kế hoạch v2
├── drivers/                      # Public peripheral API + SoC backend
├── examples/                     # 22 examples theo lộ trình 01–16
├── haievent/                     # Event framework
├── kernel/                       # Kernel public/internal/source
├── labs/memory-allocator/        # Allocator experiment, không thuộc kernel runtime
├── soc/                          # Startup/register/clock/IRQ
├── tests/                        # Host/mock/portability/stress
├── tools/                        # OpenOCD/GDB
├── CMakeLists.txt
├── Makefile
├── VERSION
└── CHANGELOG.md
```

Phân tích chi tiết từng thư mục: [`docs/00-overview/project-analysis.md`](docs/00-overview/project-analysis.md).

## 6. Build nhanh

```bash
make help
make list-targets
make list-examples
```

Build target tích hợp:

```bash
make TARGET=bluepill_f103c8 \
     EXAMPLE=16-diagnostics-stress-stabilization \
     build
```

Flash/verify/reset:

```bash
make TARGET=bluepill_f103c8 \
     EXAMPLE=16-diagnostics-stress-stabilization \
     run
```

Host example:

```bash
make TARGET=bluepill_f103c8 \
     ENVIRONMENT=host \
     EXAMPLE=02-kernel-data-structures-host \
     run
```

Host test suite:

```bash
make TARGET=bluepill_f103c8 host-tests
```

## 7. Quy ước command

```bash
make TARGET=<target> EXAMPLE=<name> build [ENVIRONMENT=host|target]
make TARGET=<target> EXAMPLE=<name> run   [ENVIRONMENT=host|target]
make TARGET=<target> EXAMPLE=<name> check [ENVIRONMENT=host|target]
make TARGET=<target> EXAMPLE=<name> clean [ENVIRONMENT=host|target]
make TARGET=<target> EXAMPLE=<name> intellisense
```

`run` trên host chạy executable; `run` trên target flash firmware. `check` chạy host tests trước rồi build example được chọn.

## 8. CMake và target manifest

CMake là nguồn dữ liệu duy nhất cho:

```text
target -> architecture + SoC + board + driver + linker + debugger
example -> modules + compile definitions
module -> source files
```

Một target mới nằm tại:

```text
cmake/targets/<target>.cmake
```

Target manifest không được chứa logic kernel. Nó chỉ binding implementation target-specific vào các contract generic.

Hướng dẫn: [`docs/04-platform/porting-new-target.md`](docs/04-platform/porting-new-target.md).

## 9. Public/internal boundary

Application bình thường chỉ nên dùng:

```c
#include "hairtos/hairtos.h"
#include "haievent/haievent.h"
#include "board.h"
```

Các header trong:

```text
kernel/internal/
haievent/internal/
```

không phải public API. Host tests và một số example nghiên cứu/benchmark có thể được CMake cấp internal include có chủ đích.

## 10. Tài liệu

Bắt đầu tại [`docs/README.md`](docs/README.md).

Các tài liệu nền tảng:

- [`docs/00-overview/project-analysis.md`](docs/00-overview/project-analysis.md): audit toàn bộ source;
- [`docs/00-overview/architecture.md`](docs/00-overview/architecture.md): kiến trúc;
- [`docs/00-overview/capability-matrix.md`](docs/00-overview/capability-matrix.md): có/chưa có;
- [`docs/01-kernel-core/README.md`](docs/01-kernel-core/README.md): kernel;
- [`docs/03-haievent/README.md`](docs/03-haievent/README.md): event framework;
- [`docs/04-platform/porting-new-target.md`](docs/04-platform/porting-new-target.md): portability;
- [`docs/06-testing-and-quality/validation-baseline.md`](docs/06-testing-and-quality/validation-baseline.md): baseline kiểm thử;
- [`docs/09-version2/README.md`](docs/09-version2/README.md): kế hoạch Version 2.

## 11. Version 2

Version 2 không phải viết lại project từ đầu. Hướng dự kiến là giữ nền tảng v1 và tập trung vào:

```text
port contract v2
+ second real hardware target
+ interrupt priority contract
+ tickless/low-power
+ HSM
+ deferred event / RTC enforcement
+ tracing/observability
+ stronger test/release process
```

Các phần nên để sau 2.0: SMP, general-purpose kernel heap, POSIX compatibility, full driver HAL và safety certification.

Roadmap chi tiết: [`docs/09-version2/roadmap.md`](docs/09-version2/roadmap.md).

## 12. Giới hạn xác nhận

Cross-build thành công chỉ chứng minh compile/link/ABI ở mức toolchain. Những mục sau vẫn phải được xác nhận trên board thật:

- exception priority và ISR timing;
- reset retention của `.noinit`;
- marker/benchmark clock;
- stack margin dưới workload thật;
- long-duration stress;
- fault injection và reset flow.

Project được phát hành theo [`LICENSE`](LICENSE).
