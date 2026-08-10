# Stress testing

## Deterministic host stress

Scheduler stress thực hiện operation sequence dài trên ready/list structures và validate sau mỗi bước.

Baseline hiện dùng 500.000 iterations.

Deterministic seed/sequence giúp bug reproducible.

## Target stress

Example 16 kết hợp:

- queue producer/consumer;
- semaphore pulse;
- mutex-protected counters;
- periodic timer;
- preemption/time slicing;
- diagnostics monitor;
- stack/invariant checks.

## Tiêu chí

- counters tiến triển;
- không order corruption;
- no invariant failure;
- stack guards valid;
- no unexpected panic;
- health reports tiếp tục.

## Soak

10-second PASS checkpoint chỉ là smoke/stress checkpoint, không phải endurance validation.

Stable release nên chạy giờ-level soak trên target thật, lưu logs và reset reason.

## Fault injection

Fault injection phải tách normal image. Sau injected fault, reset và kiểm tra retained record.

## V2

Thêm randomized/property stress cho timeout wrap, mutex graph, event ownership và HSM transitions; giữ deterministic reproduction seed.
