# Kernel roadmap Version 2

> **Status: FUTURE DESIGN.** This content is not a capability of `hairtos 1.0.0-rc1`.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](haievent-roadmap.md) · [Next →](migration-v1-to-v2.md)

## Table of Contents

- [Baseline v1](#baseline)
- [Goals](#goals)
- [Design constraints](#constraints)
- [Completion Evidence](#evidence)
- [Migration/risk](#migration)
- [References](#references)

<a id="baseline"></a>
## Baseline v1

Version 2 must start from the existing v1 behavior: static object ownership, fixed-priority scheduling, intrusive ready/wait/timeout structures, direct-handoff IPC, one task per AO, flat FSMs, target manifests, and host sanitizer tests. A “new design” must not discard useful evidence merely to change the architecture.

<a id="goals"></a>
## Goals

- Prioritize an interrupt-ceiling/BASEPRI-style contract before adding new scheduling features.
- Tickless idle requires a unified next-deadline model across task timeouts and software timers plus board/port sleep hooks.
- Do not introduce a general-purpose dynamic heap into kernel critical paths.
- Stack/task-return/invariant failures should flow through contextual diagnostics rather than an ambiguous breakpoint.
- task entry return;
- AO internal dispatch failure;
- severe internal invariant failure.
- add `HR_CFG_MAX_SYSCALL_INTERRUPT_PRIORITY` or an equivalent abstraction;
- BASEPRI critical section;
- validate ISR priorities where possible;
- documentation must distinguish kernel-aware and high-urgency ISRs;
- benchmark critical latency.
- timeout list;
- software timer;
- time slicing;
- pending IRQ;

<a id="constraints"></a>
## Design constraints

- Do not merge an API/header before the implementation and tests exist.
- Every feature must document memory cost, runtime cost, ISR implications, and failure modes.
- The generic kernel must not depend on STM32 or board registers.
- Static-first remains the default; any added dynamic behavior must be explicit, bounded, and opt-in.
- Version 2 documentation must retain the proposal label until the capability matrix, source, and tests are updated.

<a id="evidence"></a>
## Completion Evidence

A roadmap item is considered implemented only when all of the following are available:

1. source implementation in the correct module/layer;
2. corresponding unit/host tests or compile probes;
3. target evidence when the feature depends on architecture/hardware;
4. compatibility/migration note;
5. benchmark/overhead evidence when timing, RAM, or Flash are affected;
6. updated capability matrix and API documentation.

<a id="migration"></a>
## Migration / risk

The primary risk is scope creep, which can make a small RTOS difficult to audit. Migration should be feature-by-feature and opt-in, keeping v1 workloads operational for as long as practical. HSM, tickless, trace, and second-target work should be phased separately so regressions can be isolated.

<a id="references"></a>
## References

- [`../00-overview/capability-matrix.md`](../00-overview/capability-matrix.md) — baseline capability.
- [`../01-kernel-core/kernel-invariants.md`](../01-kernel-core/kernel-invariants.md) — a v1 invariant that must not be silently broken.
- [`../06-testing-and-quality/validation-baseline.md`](../06-testing-and-quality/validation-baseline.md) — evidence baseline.
- [Semantic Versioning 2.0.0](https://semver.org/)
