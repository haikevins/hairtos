# Release checklist

## Source/API

- [ ] Version/changelog đồng bộ.
- [ ] Không ghost API/placeholder.
- [ ] Public/internal boundary đúng.
- [ ] Config checks cập nhật.
- [ ] V2 planned features không bị mô tả như v1 implemented.

## Host

- [ ] GCC tests PASS.
- [ ] Clang tests PASS.
- [ ] ASan/UBSan thực sự chạy, không chỉ build.
- [ ] Deterministic stress PASS.

## Target build

- [ ] Tất cả examples build cho mọi supported target.
- [ ] GCC Arm build.
- [ ] Clang/LLD build nếu được support.
- [ ] No warnings.
- [ ] Size budget.

## Hardware

- [ ] Bare-metal UART/LED/tick.
- [ ] First task/context switch.
- [ ] Preemption/time slicing.
- [ ] ISR wake.
- [ ] Timer.
- [ ] haievent integration.
- [ ] Fault retention.
- [ ] Benchmark clock/marker.
- [ ] Soak.

## Documentation

- [ ] Markdown links.
- [ ] API signatures.
- [ ] Target list.
- [ ] Known limitations.
- [ ] Migration notes nếu breaking.

## Artifact

- [ ] Không `.git`, build cache, IntelliSense DB.
- [ ] ZIP integrity.
- [ ] SHA-256.
