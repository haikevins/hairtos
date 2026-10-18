# Changelog

> Change history for `hairtos`. Current source version: `1.0.0`.

The format follows the spirit of *Keep a Changelog* and Semantic Versioning. `Unreleased` describes changes not yet associated with a tag; `docs/09-version2` is a roadmap and does not change the current runtime version.

## [Unreleased]

No unreleased changes are recorded after `1.0.0` at this time.

## [1.0.0] - 2026-10-18

### Changed

- Promoted the validated `v1.0.0-rc2` baseline to the first stable `hairtos` release.
- Updated project version, current documentation baseline, and stable release notes.
- Kept the existing tag-driven release packaging and checksum workflow.

### Verification

- GitHub Actions host validation: PASS.
- GCC host build with AddressSanitizer and UndefinedBehaviorSanitizer: PASS.
- `ctest`: PASS, 1/1 host test target with 64 test functions in the current suite.
- Host examples 02 and 14: PASS.
- Scheduler stress example 16: PASS at 500,000 iterations.
- Project owner confirmed successful execution on the STM32F103C8T6 Blue Pill reference hardware.

### Runtime scope

- No kernel, architecture-port, board, driver, SoC, `haievent`, test, or example runtime source is changed by the stable-release promotion.
- Runtime behavior remains the validated `v1.0.0-rc2` baseline.

## [1.0.0-rc2] - 2026-10-18

### Added

- GitHub Actions host validation for the target/example registry, sanitizer-backed host tests, representative examples, and the 500,000-iteration scheduler stress workload.
- Contribution, security, pull-request, bug-report, and design-proposal repository policies.
- Tag-driven release automation that validates the version/tag relationship, reruns the host release gate, produces a source archive, generates `SHA256SUMS`, and publishes GitHub Release assets.
- Curated release notes for `v1.0.0-rc2`.

### Changed

- Redesigned the root README as a portfolio-oriented landing page with a concise architecture view, capability summary, reproducible build commands, and explicit verification boundaries.
- Re-audited the kernel, `haievent`, Cortex-M3 port, STM32F1 platform, drivers, CMake/Make, examples, tests, diagnostics, benchmark, and allocator lab documentation.
- Rewrote first-party Markdown around the structure: scope → contract → source mapping → invariant → failure semantics → validation → references.
- Clearly separated the v1 implementation (`docs/00`–`08`) from the Version 2 proposal (`docs/09-version2`).

### Verification

- Host CI: PASS.
- GCC host build with AddressSanitizer and UndefinedBehaviorSanitizer: PASS.
- `ctest`: PASS, 1/1 host test target (64 test functions in the current suite).
- Host examples 02 and 14: PASS.
- Scheduler stress example 16: PASS at 500,000 iterations.
- ARM cross-build and physical-board execution remain separate release evidence and are not implied by these host results.

### Runtime scope

- No kernel, architecture-port, board, driver, SoC, `haievent`, test, or example runtime behavior is intentionally changed by this release-preparation series.

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
