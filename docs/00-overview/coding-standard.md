# Repository Coding Standard

> **Scope:** Conventions observed in source plus compiler policy; this does not replace MISRA/CERT, and the repository does not claim certification.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](capability-matrix.md) · [Next →](configuration.md)

## Table of Contents

- [Compiler discipline](#compiler)
- [Naming/layers](#naming)
- [Types/conversions](#types)
- [Concurrency](#concurrency)
- [Error handling](#errors)
- [Data-structure discipline](#ds)
- [Documentation/test expectations](#docs)

<a id="compiler"></a>
## Compiler discipline

Target code is compiled as C11 with `-ffreestanding`, `-fno-common`, `-fno-builtin`, function/data sections, and a strict warning set: `-Wall -Wextra -Werror -Wshadow -Wundef -Wconversion -Wsign-conversion`. Host builds additionally use `-pedantic` and ASan/UBSan.

<a id="naming"></a>
## Naming and Layering

- `hr_` = hairtos kernel/public/internal symbol.
- `he_` = haievent.
- `board_` = board service.
- `stm32f1_` = SoC-specific helper.
- `HR_CFG_` / `HE_CFG_` = compile-time config.
- public headers live in public include trees; internal headers are not exposed merely for convenience.

<a id="types"></a>
## Types and Conversions

Code consistently uses `<stdint.h>`, `<stddef.h>`, `size_t`, `uintptr_t`, and `U/UL` suffixes. Conversion/sign-conversion warnings require narrowing and signed/unsigned interactions to be intentional. Pointer↔integer conversions appear only at architecture/register/binary boundaries with explicit justification.

<a id="concurrency"></a>
## Concurrency

- Critical sections save/restore the previous PRIMASK state rather than unconditionally re-enabling interrupts.
- ISR paths do not call blocking APIs.
- Shared intrusive structures are mutated under the kernel/critical-section contract.
- User callbacks do not execute in SysTick ISR context.
- Mutex ownership and effective-priority changes must be atomic with wait/ready requeue operations.

<a id="errors"></a>
## Error handling

Public operations return `hr_status_t` for recoverable contract failures. Internal invariants/faults use assert/panic/diagnostics paths. Examples use `board_panic()` to turn violations into debuggable stops rather than continuing with invalid state.

<a id="ds"></a>
## Data-structure discipline

Intrusive nodes are initialized before insertion, double insert/remove is rejected, and lists have validators. Magic values distinguish initialized opaque objects. `_Static_assert` ensures hidden control blocks fit public storage.

<a id="docs"></a>
## Documentation/test expectations

A new feature requires a public contract, source mapping, documented failure modes, appropriate host/target evidence, and an updated capability matrix. A Version 2 proposal is not considered implemented merely because a header or document exists.

## References

- [Arm Cortex-M3 Technical Reference Manual](https://developer.arm.com/documentation/100165/latest/)
- [Arm Cortex-M3 Devices Generic User Guide](https://developer.arm.com/documentation/dui0552/latest/)
