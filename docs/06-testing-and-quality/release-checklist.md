# Release checklist

> This checklist is for project release candidates; it is not a safety certification.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](kernel-benchmark.md) · [Next →](stress-testing.md)

## Source/build

- [ ] `VERSION` matches the intended changelog/tag.
- [ ] `make host-tests` passes with sanitizers enabled.
- [ ] Strict warnings are not disabled merely to make the build pass.
- [ ] Target build passes with the supported toolchain.
- [ ] `make list-targets`/`list-examples` matches the CMake registry.
- [ ] Public/internal include boundaries have not been globally relaxed.

## Kernel/framework correctness

- [ ] Ready/wait/timeout validators PASS.
- [ ] Tick-wrap tests PASS.
- [ ] Queue/semaphore/mutex/timer tests PASS.
- [ ] Priority inheritance regression tests PASS.
- [ ] `haievent` refcount/FSM/pubsub tests PASS.
- [ ] Retained diagnostics records are verified on target when the release advertises this feature.

## Target evidence

- [ ] Startup/SVC/PendSV examples run on the board.
- [ ] UART/LED/clock bindings match the target manifest.
- [ ] Fault injection and post-reset records are verified when diagnostics are enabled.
- [ ] Benchmarks are remeasured if toolchain/configuration/clock changes.

## Documentation

- [ ] The capability matrix marks a feature **Yes** only when source/evidence exists.
- [ ] API documentation matches public headers.
- [ ] Version 2 documentation remains labeled as future design.
- [ ] Markdown links and Mermaid diagrams render cleanly.
- [ ] Changelog documents breaking or behavioral changes.

## Packaging

- [ ] Do not include `.git`, build trees, generated compilation databases, or caches in release archives unless intentionally required.
- [ ] License, VERSION, and README exist at the repository root.
