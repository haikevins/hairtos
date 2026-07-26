# Changelog

All notable changes to `hairtos` are documented in this file.

The format follows Keep a Changelog and the project uses Semantic Versioning.

## [Unreleased]

### Changed

- Rewrote every `examples/*/README.md` in Vietnamese with one consistent
  ten-section structure covering goals, configuration, execution flow, APIs,
  build commands, expected output, PASS criteria, troubleshooting and scope.
- Rebuilt `examples/README.md` as the central host/target example index.

## [1.0.0-rc1] - 2026-07-27

### Added

- Static-first Cortex-M3 kernel with task, scheduler, timeout, queue, semaphore,
  mutex, software timer, diagnostics and retained fault records.
- `haievent` event framework with Active Objects, flat state machines, time
  events and publish/subscribe.
- Host sanitizer tests, deterministic scheduler stress, allocator lab and DWT
  benchmark examples.
- Root `VERSION` file and this changelog.

### Changed

- Made CMake the single source of truth for example/module/source selection.
- Reduced the root Makefile to a consistent command-line wrapper around CMake.
- Restricted `kernel/internal` and `haievent/internal` include paths to kernel,
  framework, tests and the two examples that intentionally inspect internals.
- Reorganized drivers into public interfaces, common code and SoC-specific
  implementations so a second SoC can be added without mixing implementations.

### Removed

- Duplicate benchmark placeholder under `tests/benchmark/`.
- Obsolete repository-hygiene and package-release documentation.
