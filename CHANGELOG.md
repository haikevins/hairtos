# Changelog

> Change history for `hairtos`. Current source version: `1.0.0-rc1`.

The format follows the spirit of *Keep a Changelog* and Semantic Versioning. `Unreleased` describes changes not yet associated with a tag; `docs/09-version2` is a roadmap and does not change the current runtime version.

## [Unreleased]

### Documentation

- Re-audited the kernel, `haievent`, Cortex-M3 port, STM32F1 platform, drivers, CMake/Make, examples, tests, diagnostics, benchmark, and allocator lab.
- Rewrote all first-party Markdown around the structure: scope → contract → source mapping → invariant → failure semantics → validation → references.
- Clearly separated the v1 implementation (`docs/00`–`08`) from the Version 2 proposal (`docs/09-version2`).
- Added navigation/TOC/Mermaid using appropriate semantics and established a validation baseline from the host test suite.

## [1.0.0-rc1] - 2026-07-27

### Added

- Static-first Cortex-M3 kernel: tasks, scheduler, timeouts, queues, semaphores, mutexes, software timers, diagnostics, and a retained fault record.
- `haievent`: Active Object, flat state machine, time events, fixed-block event pool, and publish/subscribe.
- Host sanitizer tests, deterministic scheduler stress tests, allocator lab, and target benchmark.
- Target manifest separates architecture/SoC/board/driver selection from the generic build graph.
- `VERSION` and changelog.

### Changed

- CMake became the source of truth for target/example/module/source selection; Makefile became a command wrapper.
- `kernel/internal` and `haievent/internal` are exposed only to tests/examples/modules with explicit justification.
- Public driver APIs and the STM32F1 backend were separated.
- The kernel tick IRQ was moved into the architecture layer.
- Benchmark clock/board markers were separated from generic statistics.

### Removed

- Duplicate benchmark placeholder.
- Skeleton/documentation no longer participates in the runtime build.

## References

- [Semantic Versioning 2.0.0](https://semver.org/)
