# Release checklist

> This checklist is for project releases; it is not a safety certification.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](kernel-benchmark.md) · [Next →](stress-testing.md)

## Source/build

- [ ] `VERSION` matches the intended changelog entry and Git tag (`v$(cat VERSION)`).
- [ ] `make host-tests` passes with sanitizers enabled.
- [ ] Strict warnings are not disabled merely to make the build pass.
- [ ] Target build passes with the supported toolchain when target validation is part of the release evidence.
- [ ] `make list-targets`/`list-examples` matches the CMake registry.
- [ ] Public/internal include boundaries have not been globally relaxed.

## Kernel/framework correctness

- [ ] Ready/wait/timeout validators PASS.
- [ ] Tick-wrap tests PASS.
- [ ] Queue/semaphore/mutex/timer tests PASS.
- [ ] Priority inheritance regression tests PASS.
- [ ] `haievent` refcount/FSM/pubsub tests PASS.
- [ ] Retained diagnostics records are verified on target when the release advertises target evidence for this feature.

## Target evidence

- [ ] Startup/SVC/PendSV examples run on the board when hardware validation is claimed.
- [ ] UART/LED/clock bindings match the target manifest.
- [ ] Fault injection and post-reset records are verified when target diagnostics evidence is claimed.
- [ ] Benchmarks are remeasured if toolchain/configuration/clock changes.
- [ ] Host-only validation is not presented as ARM cross-build or physical-board evidence.

## Documentation

- [ ] The capability matrix marks a feature **Yes** only when source/evidence exists.
- [ ] API documentation matches public headers.
- [ ] Version 2 documentation remains labeled as future design.
- [ ] Markdown links and Mermaid diagrams render cleanly.
- [ ] Changelog documents breaking or behavioral changes.
- [ ] `docs/06-testing-and-quality/releases/v$(cat VERSION).md` exists and matches the intended release scope.

## Packaging

- [ ] Do not include `.git`, build trees, generated compilation databases, or caches in release archives unless intentionally required.
- [ ] License, `VERSION`, README, changelog, and release notes exist in the release source archive.
- [ ] The tag-driven release workflow validates tag/version agreement before publishing.
- [ ] The release archive has a matching entry in `SHA256SUMS` and the checksum verifies successfully.
- [ ] A release candidate is marked as a GitHub prerelease; a stable version is not.
