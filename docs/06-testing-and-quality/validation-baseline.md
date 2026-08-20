# Validation baseline

> **Source baseline:** `1.0.0-rc1`.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](testing-guide.md)

## Kết quả đã chạy

| Validation | Kết quả |
| --- | --- |
| CMake host test configure/build | PASS |
| GCC strict warnings | PASS |
| AddressSanitizer/UBSan host binary | PASS |
| `ctest` | PASS, 1/1 test target |
| Host test functions | 64 |
| Example 02 host | PASS |
| Example 14 allocator host | PASS |
| Example 16 scheduler stress host | PASS |
| Stress iterations | 500,000 |

## Stress result quan sát

```text
hairtos scheduler stress: PASS
iterations=500000
insertions=83266
removals=83262
rotations=164652
validations=500000
```

## Environment limitation

Host validation dùng native GCC/CMake/Ninja. Target build và hardware validation là evidence riêng, cần ARM GNU toolchain, GDB/OpenOCD và board thật trước khi được đánh dấu PASS.

## Ý nghĩa

Host evidence cho thấy generic C policy/data structure và framework tests không phát hiện memory UB theo sanitizer trong workload hiện tại. Nó **không** chứng minh WCET, interrupt latency, electrical behavior hay fault retention qua reset thật.

## Reproduce

```bash
make TARGET=bluepill_f103c8 host-tests
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=02-kernel-data-structures-host run
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run
```
