# Flat state machine

## 1. Mục tiêu

Dispatch event theo run-to-completion và hỗ trợ transition có ENTRY/EXIT/INIT.

## 2. State handler

```c
he_state_result_t state(he_state_machine_t *machine,
                        const he_event_t *event);
```

Handler trả `HANDLED`, `IGNORED` hoặc `TRANSITION`.

## 3. Khởi tạo

`he_state_machine_init()` đặt initial handler và context. `he_state_machine_start()` thực hiện ENTRY/INIT cần thiết và đánh dấu started.

## 4. Dispatch

Mỗi event được handler hiện tại xử lý hoàn toàn trước event tiếp theo. Không block bên trong state handler nếu muốn giữ latency dự đoán được.

## 5. Transition

```text
old state --HE_SIG_EXIT--> đổi current state --HE_SIG_ENTRY--> new state
                                                    |
                                             HE_SIG_INIT loop
```

`HE_CFG_MAX_INIT_TRANSITIONS` giới hạn chuỗi init để tránh loop lỗi.

## 6. Context

State machine lưu `void *context`; helper trả mutable hoặc const pointer. Context thường là struct application chứa state data và reference tới AO/timer.

## 7. Invariants

- Machine phải valid và started trước dispatch.
- Target transition không NULL.
- Không dispatch reentrant cùng machine.

## 8. Giới hạn

Chỉ flat state machine; `HE_CFG_ENABLE_HIERARCHICAL_SM` đang bằng 0.
