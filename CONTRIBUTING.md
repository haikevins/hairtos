# Contributing to hairtos

Thank you for taking the time to improve `hairtos`.

`hairtos` is an educational and experimental RTOS project. Contributions should preserve the distinction between implemented v1 behavior and future design material under `docs/09-version2/`, and should avoid claiming target or hardware validation that was not actually executed.

## Before you start

- Check the existing documentation under [`docs/`](docs/README.md) and examples under [`examples/`](examples/README.md).
- Keep changes focused. Avoid mixing kernel behavior changes, platform changes, documentation cleanup, and unrelated refactors in one pull request.
- Prefer static ownership and the existing public/internal boundaries unless the change explicitly proposes a different design.
- Do not move roadmap-only v2 concepts into current-runtime documentation without an implementation change that justifies it.

## Build and validation

List the registered targets and examples first:

```bash
make list-targets
make list-examples
```

Run the host validation baseline:

```bash
make TARGET=bluepill_f103c8 host-tests
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=02-kernel-data-structures-host run
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=14-memory-allocator-lab run
make TARGET=bluepill_f103c8 ENVIRONMENT=host EXAMPLE=16-diagnostics-stress-stabilization run
```

Host validation is not a substitute for target validation. If a change affects the Cortex-M port, startup, linker script, SoC, board support, interrupt behavior, or hardware-facing drivers, also perform the relevant ARM cross-build and physical-board checks before marking those areas as validated.

For the current reference target, a representative target build is:

```bash
make TARGET=bluepill_f103c8 EXAMPLE=16-diagnostics-stress-stabilization build
```

See [`docs/06-testing-and-quality/`](docs/06-testing-and-quality/README.md) for the testing model and validation baseline.

## Change expectations

A good change should:

- compile cleanly under the validation path it affects;
- keep warnings meaningful instead of suppressing them without justification;
- preserve scheduler, timeout, ownership, and synchronization invariants;
- add or update tests when behavior changes;
- update documentation when public behavior, configuration, target support, or limitations change;
- keep current implementation claims separate from future proposals;
- avoid unrelated generated files or build artifacts.

## Commit and pull request style

Use concise imperative commit subjects with a clear area, for example:

```text
fix(kernel): preserve waiter ordering after timeout
feat(haievent): add static event helper
docs(platform): clarify PendSV context ownership
test(queue): cover timeout wrap-around
```

A pull request should explain:

1. what changed and why;
2. which subsystem is affected;
3. which validation commands were actually run;
4. whether target or physical hardware was exercised;
5. any known limitations or follow-up work.

The repository pull request template contains the same checklist.

## Documentation-only changes

Documentation changes should still run `git diff --check`. If they modify commands, validation claims, configuration, supported targets, or API behavior, verify those statements against the current source before submitting.

## Reporting security issues

Do not publish exploit details for a suspected security issue in a public issue. Follow [`SECURITY.md`](SECURITY.md) instead.
