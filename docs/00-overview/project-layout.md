# Cấu trúc repository

## 1. Mục tiêu

Tài liệu này giải thích vai trò của từng thư mục và nơi nên đặt code mới.

## 2. Cây thư mục chính

```text
hairtos/
├── arch/                  # CPU architecture port
├── benchmarks/            # Benchmark code chỉ dùng khi được chọn
├── boards/                # Board-specific clock, pin, linker
├── cmake/                 # Source-of-truth build modules and toolchains
├── config/                # hairtos và haievent configuration
├── docs/                  # Tài liệu đã phân loại
├── drivers/               # Public interfaces, common code, SoC implementations
├── examples/              # Host và target demonstrations
├── haievent/             # Event-driven framework
├── kernel/                # Public API, internal headers, implementation
├── labs/                  # Các lab độc lập khỏi kernel runtime
├── soc/                   # STM32F1 register/startup/IRQ support
├── tests/                 # Host unit tests và stress tests
└── tools/                 # OpenOCD, GDB, validation, packaging
```

## 3. Quy tắc đặt file

| Loại code | Vị trí |
|---|---|
| Public kernel API | `kernel/include/hairtos/` |
| Kernel-only type/function | `kernel/internal/` |
| Kernel implementation | `kernel/src/` |
| Public haievent API | `haievent/include/haievent/` |
| haievent internal layout | `haievent/internal/` |
| CPU port | `arch/<architecture>/` |
| MCU family | `soc/<family>/` |
| Board | `boards/<board>/` |
| Public driver API | `drivers/include/` |
| Architecture tick adapters | `arch/<architecture>/` |
| SoC driver implementation | `drivers/<soc>/` |
| Independent experiment | `labs/<name>/` |
| User-facing demo | `examples/<number-name>/` |
| Host unit test | `tests/host/` |
| Long-running deterministic test | `tests/stress/` |

## 4. Public và private headers

Application chỉ nên thêm include path `kernel/include` và `haievent/include`. `kernel/internal` chỉ được dùng bởi kernel, port, host tests và benchmark cần kiểm tra internal policy.

Opaque public objects như `hr_task_t` hoặc `hr_queue_t` chứa static byte storage. Layout thật được giữ trong internal headers và được bảo vệ bằng `_Static_assert`.

## 5. Example numbering

Tên example bám theo roadmap. Phase 10 và Phase 13 có nhiều example nên sử dụng sub-number như `10-01-*` và `13-06-*`. `02-kernel-data-structures-host` là host-only; các example còn lại trong danh sách Makefile là target.

## 6. Không nên tạo lại

Không thêm skeleton rỗng, ghost API hoặc file không tham gia build. Mỗi source mới phải được một trong các thành phần sau sử dụng: Makefile, CMake, host tests, stress tests, validation tool hoặc tài liệu rõ ràng.
## 7. Build source of truth

`cmake/hairtos_examples.cmake` maps examples to modules and feature definitions.
`cmake/hairtos_modules.cmake` owns module-to-source mappings. The root Makefile
never repeats these lists; it configures and invokes CMake.

This arrangement prevents Make and CMake from selecting different source sets.

