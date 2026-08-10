# API compatibility policy cho Version 2

## Nguyên tắc

Không phá v1 API chỉ để đổi tên thẩm mỹ.

Breaking change chỉ hợp lý nếu:

- sửa semantic bug;
- mở đường HSM/port contract không thể làm sạch bằng extension;
- loại contract nguy hiểm;
- giảm coupling lớn.

## Kernel API

Ưu tiên giữ:

```text
hr_task_*
hr_queue_*
hr_semaphore_*
hr_mutex_*
hr_timer_*
hr_context_*
```

Nếu thêm ISR priority contract, public ISR function signature chỉ đổi khi thực sự cần; ưu tiên config/port validation.

## haievent

Flat state machine API có thể giữ compatibility wrapper trên HSM core.

Dynamic event ownership semantics không được âm thầm đổi.

## Opaque storage

Internal object có thể lớn hơn ở v2. Nếu public storage size tăng, application static RAM tăng nhưng source vẫn compile. Cần ghi footprint change.

## Deprecation

Nếu API cũ cần bỏ:

1. mark deprecated macro/comment;
2. docs migration;
3. compatibility window nếu chi phí thấp;
4. remove ở major version phù hợp.

## Semantic versioning

- 1.x: compatibility-focused stabilization;
- 2.0: controlled breaking changes;
- 2.x: backward-compatible features/fixes theo public contract mới.
