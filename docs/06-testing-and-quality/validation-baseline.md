# Validation baseline

> **Source baseline:** `1.0.0-rc1`.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](testing-guide.md)

## Executed Results

| Validation | Result |
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

## Observed Stress Result

```text
hairtos scheduler stress: PASS
iterations=500000
insertions=83266
removals=83262
rotations=164652
validations=500000
```

## Environment limitation

Host validation uses native GCC/CMake/Ninja. Target build and hardware validation are separate evidence and require the ARM GNU toolchain, GDB/OpenOCD, and a physical board before being marked PASS.

## Interpretation

Host evidence shows that the current workload did not expose memory undefined behavior in the generic C policy/data-structure and framework tests under sanitizers. It **does not** prove WCET, interrupt latency, electrical behavior, or retained-fault behavior across a physical reset.

## Reproduce

```bash
make TARGET=bluepill_f103c8 host-tests
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=02-kernel-data-structures-host run
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run
```
