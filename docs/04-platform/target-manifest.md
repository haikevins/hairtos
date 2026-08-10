# Target manifest

## Mục tiêu

`cmake/targets/<target>.cmake` là điểm binding duy nhất giữa build generic và hardware implementation.

## Discovery

`cmake/hairtos_targets.cmake` tìm các target manifest và validate required fields/files.

## Target hiện tại

```text
bluepill_f103c8
```

Manifest khai báo:

```text
target name/description
architecture/SoC/board identity
CPU flags
target compile definitions
public include paths
platform C sources
startup ASM
port C/ASM
bare-metal tick adapter
kernel tick adapter
fault C/ASM
benchmark clock backend
linker script
OpenOCD config
erase command
```

## Vì sao manifest tốt?

CMake root không cần biết STM32F1. Module mapping chỉ dùng abstract variables từ target.

Khi target khác dùng cùng Cortex-M3 port, manifest mới có thể reuse architecture sources nhưng thay SoC/board/driver/linker.

## Validation

Manifest loader nên fail sớm nếu:

- file declared không tồn tại;
- target name mismatch;
- required source list rỗng;
- linker/OpenOCD path invalid;
- manifest version không hỗ trợ.

## Hạn chế v1

Manifest đang flat. Khi có nhiều board cùng SoC hoặc nhiều SoC cùng arch, các list architecture/SoC có thể bị copy lặp giữa target manifests.

Version 2 nên cân nhắc reusable `arch`, `soc`, `board` fragments hoặc helper function, nhưng không cần over-engineer trước target thứ hai.
