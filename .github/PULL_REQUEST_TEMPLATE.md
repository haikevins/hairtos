## Summary

<!-- What changed, and why? Keep the scope focused. -->

## Affected area

<!-- Mark the relevant areas. -->

- [ ] Kernel / scheduler / timeout
- [ ] IPC / synchronization
- [ ] `haievent`
- [ ] Cortex-M architecture port
- [ ] SoC / board / drivers / linker / startup
- [ ] Build / tooling / CI
- [ ] Tests / benchmarks
- [ ] Documentation only

## Validation performed

<!-- Check only commands/evidence that were actually executed. -->

- [ ] `git diff --check`
- [ ] `make TARGET=bluepill_f103c8 host-tests`
- [ ] Example 02 host validation
- [ ] Example 14 allocator validation
- [ ] Example 16 scheduler stress validation
- [ ] ARM target cross-build
- [ ] Physical Blue Pill validation
- [ ] Not applicable (explain below)

Validation notes / commands:

```text

```

## Hardware evidence

<!-- Do not infer target/hardware PASS from host CI. -->

- Target/toolchain used:
- Board/debug probe used:
- Hardware behavior observed:
- Not applicable because:

## Checklist

- [ ] The change is focused and does not include unrelated build artifacts.
- [ ] Tests or documentation were updated when behavior changed.
- [ ] Current v1 behavior and `docs/09-version2/` roadmap material remain clearly separated.
- [ ] Validation claims in this PR match evidence that was actually collected.
- [ ] Public/API/configuration changes document any new limitation or compatibility impact.
