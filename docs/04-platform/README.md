# 04 — Platform, architecture port và target

Đây là ranh giới giữa kernel generic và hardware.

## Tài liệu

- [port-contract.md](port-contract.md)
- [target-manifest.md](target-manifest.md)
- [cortex-m3-port.md](cortex-m3-port.md)
- [stm32f103-platform.md](stm32f103-platform.md)
- [startup-and-linker.md](startup-and-linker.md)
- [drivers.md](drivers.md)
- [porting-guide.md](porting-guide.md)
- [porting-new-target.md](porting-new-target.md)

## Phân lớp

```text
arch   = CPU/ISA
soc    = MCU family
board  = concrete board/memory/pins
driver = peripheral implementation
target manifest = binding
```

Một target mới không được yêu cầu sửa `kernel/` hoặc `haievent/`.
