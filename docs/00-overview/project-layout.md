# Cấu trúc repository và trách nhiệm

## Root

`Makefile` là CLI wrapper. `CMakeLists.txt` tạo object targets và link firmware/host executable. `VERSION` là version hiện tại; `CHANGELOG.md` ghi thay đổi.

## `kernel/`

```text
include/hairtos/  public API
internal/         internal layout/contracts
src/              generic implementation
```

Không đặt register access hoặc board pin trong kernel.

## `haievent/`

```text
include/haievent/ public framework API
internal/         control-block layouts
src/              event/FSM/AO/time/pubsub
```

Chỉ phụ thuộc public kernel API và config.

## `arch/`

CPU/ISA-specific:

- initial stack;
- critical section;
- SVC/PendSV;
- fault entry;
- tick IRQ adapter;
- benchmark cycle clock.

Một MCU mới dùng cùng ISA có thể tái sử dụng phần lớn folder này.

## `soc/`

MCU-family:

- register definitions;
- startup vector;
- clock setup;
- IRQ support/fallback.

## `boards/`

Board-level binding:

- LED/UART/pins;
- board name/CPU name;
- panic behavior;
- benchmark marker;
- linker script;
- flash/static RAM footprint hooks.

## `drivers/`

`drivers/include` là API generic. `drivers/<soc>` chứa identifier encoding và register-level implementation.

## `cmake/targets/`

Mỗi `.cmake` là một hardware target manifest. Đây là nơi binding arch + SoC + board + driver + linker + OpenOCD.

## `tests/`

```text
host/         unit/regression tests
mocks/        fake architecture port
portability/  compile proof
stress/       deterministic long operation sequences
```

## `examples/`

22 executable examples. Folder number thể hiện lộ trình học, không phải tên API nội bộ.

## `labs/`

Experiment không phải runtime dependency. Hiện có memory allocator lab.

## `benchmarks/`

Benchmark statistics generic; clock backend nằm ở architecture layer.

## `docs/`

00–08 mô tả v1. 09 mô tả v2 planned.

## Quy tắc thêm file

Một source mới phải trả lời được:

1. thuộc layer nào?
2. được build bởi module nào?
3. public hay internal?
4. host-test được không?
5. target-specific assumption nằm ở đâu?
6. tài liệu nào mô tả contract của nó?
