# Release checklist

> Checklist này dùng cho release candidate của project, không phải chứng nhận safety.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](kernel-benchmark.md) · [Next →](stress-testing.md)

## Source/build

- [ ] `VERSION` khớp changelog/tag dự kiến.
- [ ] `make host-tests` PASS với sanitizer.
- [ ] Strict warnings không bị tắt để “qua build”.
- [ ] Target build bằng toolchain supported PASS.
- [ ] `make list-targets`/`list-examples` phản ánh đúng CMake registry.
- [ ] Public/internal include boundary không bị nới toàn cục.

## Kernel/framework correctness

- [ ] Ready/wait/timeout validators PASS.
- [ ] Tick-wrap tests PASS.
- [ ] Queue/semaphore/mutex/timer tests PASS.
- [ ] Priority inheritance regression tests PASS.
- [ ] `haievent` refcount/FSM/pubsub tests PASS.
- [ ] Diagnostics retained record behavior được kiểm target nếu release quảng bá feature này.

## Target evidence

- [ ] Startup/SVC/PendSV examples chạy trên board.
- [ ] UART/LED/clock binding đúng target manifest.
- [ ] Fault injection/reset record được kiểm nếu diagnostics on.
- [ ] Benchmark được đo lại nếu toolchain/config/clock đổi.

## Documentation

- [ ] Capability matrix chỉ đánh dấu **Có** khi có source/evidence.
- [ ] API docs khớp public headers.
- [ ] Version 2 docs vẫn gắn nhãn future.
- [ ] Markdown links/Mermaid render sạch.
- [ ] Changelog mô tả breaking/behavior change.

## Packaging

- [ ] Không đưa `.git`, build tree, generated compile database/cache vào release archive nếu không có chủ đích.
- [ ] License/VERSION/README tồn tại ở root.
