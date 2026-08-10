# Version 2 — Kế hoạch tương lai

> **Trạng thái:** planned. Không có nội dung nào trong thư mục này được coi là capability của `hairtos 1.0.0-rc1`.

Version 2 không nhằm "thêm thật nhiều API". Mục tiêu là làm architecture v1 mạnh hơn ở bốn điểm:

```text
Correctness
Portability
Event modeling
Observability / validation
```

## Các trụ cột

1. [vision-and-goals.md](vision-and-goals.md) — mục tiêu và non-goals.
2. [architecture.md](architecture.md) — architecture dự kiến.
3. [kernel-roadmap.md](kernel-roadmap.md) — port/interrupt/time/sync.
4. [haievent-roadmap.md](haievent-roadmap.md) — HSM, RTC, deferred event.
5. [portability-roadmap.md](portability-roadmap.md) — second target và target schema.
6. [diagnostics-and-observability.md](diagnostics-and-observability.md) — trace.
7. [testing-and-release.md](testing-and-release.md) — CI/hardware/release.
8. [api-compatibility.md](api-compatibility.md) — compatibility rules.
9. [migration-v1-to-v2.md](migration-v1-to-v2.md) — migration.
10. [risk-register.md](risk-register.md) — rủi ro scope/architecture.
11. [roadmap.md](roadmap.md) — milestones cụ thể.

## Definition of Version 2

Một `2.0.0` đáng phát hành khi:

- v1 public use-cases vẫn migration rõ;
- actual second MCU/board target chạy kernel + haievent;
- interrupt priority contract rõ và test được;
- HSM core ổn định;
- blocking trong AO dispatch được detect trong debug configuration;
- tickless/low-power contract hoạt động ít nhất trên một target;
- trace ring giúp reconstruct scheduling/event sequence;
- host/target CI matrix + hardware regression đủ mạnh;
- docs phân biệt capability/limitation chính xác.

Các feature như SMP, POSIX, full HAL, safety certification không phải 2.0 goal.
