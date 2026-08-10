# 06 — Testing, diagnostics và chất lượng

Chất lượng của RTOS không thể dựa vào một demo blink. `hairtos` tách kiểm thử thành nhiều tầng:

```text
host unit tests
 -> deterministic stress
 -> target cross-build
 -> target runtime examples
 -> fault injection
 -> long-duration soak
 -> benchmark/timing
```

## Tài liệu

- [testing-guide.md](testing-guide.md)
- [validation-baseline.md](validation-baseline.md)
- [test-matrix.md](test-matrix.md)
- [diagnostics.md](diagnostics.md)
- [stress-testing.md](stress-testing.md)
- [kernel-benchmark.md](kernel-benchmark.md)
- [release-checklist.md](release-checklist.md)

Không gọi một release "validated" nếu chỉ cross-build mà chưa chạy phần hardware-dependent cần thiết.
