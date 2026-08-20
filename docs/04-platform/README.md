# 04 — Platform, architecture port và target

> **Vai trò:** Trang này là index của một cụm tài liệu. Mỗi chapter đi từ bản chất → mapping source → invariant → failure mode → validation để người đọc có thể đối chiếu trực tiếp với implementation.

[← Root README](../../README.md)

## Mục lục

- [Bản đồ nội dung](#ban-do)
- [Cách đọc](#cach-doc)
- [Các tài liệu](#tai-lieu)
- [Validation baseline](#validation)
- [Tài liệu tham khảo](#references)

<a id="ban-do"></a>
## Bản đồ nội dung

```mermaid
flowchart LR
    INDEX["Index"] --> CONCEPT["Concept / contract"]
    CONCEPT --> SOURCE["Source mapping"]
    SOURCE --> TEST["Validation / evidence"]
```

<a id="cach-doc"></a>
## Cách đọc

1. Bắt đầu từ README của section để biết scope và thứ tự học.
2. Khi gặp API, quay lại `docs/05-api-reference/` để xem context/return contract; khi gặp behavior kernel, ưu tiên `docs/01`–`03`.
3. Đối chiếu mọi statement timing/ownership với source map ở cuối chapter.
4. Phân biệt rõ **host evidence**, **target evidence** và **future proposal**.

<a id="tai-lieu"></a>
## Các tài liệu

| Tài liệu | Vai trò |
| --- | --- |
| [`cortex-m3-port.md`](cortex-m3-port.md) | ARM Cortex-M3 port |
| [`drivers.md`](drivers.md) | Driver abstraction |
| [`port-contract.md`](port-contract.md) | Architecture port contract |
| [`porting-guide.md`](porting-guide.md) | Porting guide — tư duy |
| [`porting-new-target.md`](porting-new-target.md) | Port một target MCU mới |
| [`startup-and-linker.md`](startup-and-linker.md) | Startup và linker contract |
| [`stm32f103-platform.md`](stm32f103-platform.md) | STM32F103 / Blue Pill target |
| [`target-manifest.md`](target-manifest.md) | Target manifest |

<a id="validation"></a>
## Validation baseline

- `VERSION`: `1.0.0-rc1`.
- Host test suite hiện có 64 test function và đã chạy PASS trong lần audit tài liệu này.
- `02-kernel-data-structures-host`, `14-memory-allocator-lab`, `16-diagnostics-stress-stabilization` chạy PASS trên host.
- Target tham chiếu là `bluepill_f103c8`; cross toolchain/OpenOCD không có trong môi trường audit nên không tuyên bố đã flash lại hardware.

<a id="references"></a>
## Tài liệu tham khảo

- [ST RM0008 — STM32F10x Reference Manual](https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf)
- [ST PM0056 — STM32F10xxx Cortex-M3 Programming Manual](https://www.st.com/resource/en/programming_manual/cd00228163-stm32f10xxx20xxx21xxxl1xxxx-cortexm3-programming-manual-stmicroelectronics.pdf)
- [STM32F103 documentation portal](https://www.st.com/en/microcontrollers-microprocessors/stm32f103/documentation.html)

**Nguồn implementation trong repository:**
- `README.md`
- `CMakeLists.txt`
- `cmake/hairtos_examples.cmake`
- `cmake/hairtos_targets.cmake`
