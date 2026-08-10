# Portability roadmap Version 2

## P1 — Target thứ hai

Đây là mục tiêu bắt buộc vì abstraction chỉ được chứng minh bằng reuse thật.

Target thứ hai nên tạo đủ khác biệt để lộ assumption ẩn. Hai lựa chọn hợp lý:

- Cortex-M4/M4F MCU: reuse ARM exception model nhưng test FPU/priority differences;
- Cortex-M0+: ép viết port assembly khác và test feature capability.

Không cần chọn target chỉ vì phổ biến; chọn target giúp kiểm chứng architecture boundary.

## P2 — Manifest schema v2

v1 manifest flat. Khi target count tăng, tách reusable fragments:

```text
architecture definition
SoC definition
board definition
target binding
```

Ví dụ:

```text
cmake/architectures/cortex-m3.cmake
cmake/socs/stm32f1.cmake
cmake/boards/bluepill_f103c8.cmake
cmake/targets/bluepill_f103c8.cmake
```

Chỉ thực hiện nếu target thứ hai cho thấy duplication thật.

## P3 — Capability metadata

Target/port nên expose:

```text
has benchmark clock
has marker
has retained RAM
has kernel-aware IRQ priority model
supports tickless
supports FPU context
supports MPU
```

Example có thể skip/adapt thay vì compile fail vì assumption target-specific.

## P4 — Example adapters

Examples 10-01/13-01 hiện dùng STM32F1 EXTI trực tiếp để demo ISR.

v2 có thể tạo board demo IRQ service:

```text
board_demo_irq_init()
board_demo_irq_trigger()
```

hoặc capability-specific example backend. Mục tiêu không phải generic EXTI HAL, chỉ loại direct SoC dependency khỏi educational application.

## P5 — Template cleanup

Target template phải phản ánh automatic discovery đúng, không còn comment cũ yêu cầu sửa supported-target list thủ công.

## P6 — Port conformance suite

Thêm common runtime tests:

```text
initial stack ABI
first task
register preservation
nested critical state
ISR detection
preemption
tick advancement
fault frame
benchmark clock monotonicity
```

## P7 — Documentation

Mỗi target mới có:

```text
target README
memory map
clock assumptions
debug wiring
validated toolchains
known errata/limitations
runtime evidence
```
