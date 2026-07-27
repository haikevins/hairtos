# 06 — Kiểm thử và chất lượng

## 1. Mục tiêu

Nhóm này mô tả host tests, diagnostics, stress, benchmark và release validation.

## 2. Nội dung

- [testing-guide.md](testing-guide.md)
- [diagnostics.md](diagnostics.md)
- [stress-testing.md](stress-testing.md)
- [kernel-benchmark.md](kernel-benchmark.md)
- [release-checklist.md](release-checklist.md)

## 3. Các lớp kiểm tra

- host unit tests và sanitizer;
- deterministic scheduler stress;
- cross-build theo target manifest;
- runtime smoke test trên board;
- benchmark/logic-analyzer validation;
- retained fault và long-duration soak test.

## 4. Portability

Target mới phải chạy lại toàn bộ tầng kiểm tra phù hợp. Cross-build PASS không thay thế hardware runtime validation.

## 5. Lệnh chính

```bash
make TARGET=bluepill_f103c8 host-tests
make TARGET=bluepill_f103c8 EXAMPLE=16-diagnostics-stress-stabilization check
```
