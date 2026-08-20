# Roadmap Version 2

> **Status: FUTURE DESIGN.** This content is not a capability of `hairtos 1.0.0-rc1`.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](risk-register.md) · [Next →](testing-and-release.md)

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

- Dependency-ordered phases: correctness/interrupt contract → observability → HSM core → portability target 2 → tickless → stabilization/release.
- Every phase must define problem/contract/memory/runtime/ISR/failure/tests/migration/docs before merge.
- freeze v1 behavior;
- documentation baseline complete;
- stable host-test invocation;
- hardware baseline Blue Pill;
- known issues list.
- failure policy;
- port capability cleanup;
- BASEPRI path Cortex-M3;
- max syscall IRQ priority;
- conformance tests;
- target template cleanup.
- select a target;

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
