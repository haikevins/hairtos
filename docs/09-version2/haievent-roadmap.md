# haievent roadmap Version 2

> **Status: FUTURE DESIGN.** This content is not a capability of `hairtos 1.0.0-rc1`.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](diagnostics-and-observability.md) · [Next →](kernel-roadmap.md)

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

- HSM support requires superstate relationships, parent propagation, LCA transitions, nested initial states, and model-based tests.
- Deferred-event/defer-recall support requires explicit ownership so events do not leak or double-release across state exits/entries.
- A shared executor is not the default production model for 2.0; one-task-per-AO remains the easier-to-reason-about baseline.
- each state has a parent;
- unhandled events may propagate to the parent;
- source and target states may be at different hierarchy levels;
- execute the EXIT chain up to the LCA;
- execute the ENTRY chain from the LCA down to the target;
- initial substate;
- bounded transition chain.
- self transition;
- sibling;
- child→parent;
- parent→child;
- cross-branch;

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
