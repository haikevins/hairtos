# Flat State Machine

## API model

State handler:

```c
he_state_result_t state(he_state_machine_t *machine,
                        const he_event_t *event);
```

Kết quả:

```text
HE_STATE_HANDLED
HE_STATE_IGNORED
HE_STATE_TRANSITION
```

## Start

FSM init lưu initial handler/context. Start gửi ENTRY và theo INIT transitions tối đa `HE_CFG_MAX_INIT_TRANSITIONS`.

## Dispatch

Normal event được đưa vào current state handler. Handler có thể:

- handled;
- ignored;
- request transition.

## Transition

v1 flat transition:

```text
current EXIT
 -> current = target
 -> target ENTRY
 -> follow INIT if requested
```

Không có parent state/LCA algorithm.

## Context

FSM có `void *context` để state handlers dùng shared application data mà không cần global variables.

## Guard chống loop

INIT chain có compile-time maximum. Nếu state machine cấu hình cycle init vô hạn, framework không loop mãi.

## Chưa có

- hierarchical parent/superstate;
- bubbling event lên parent;
- shallow/deep history;
- orthogonal regions;
- transition guards/actions là object riêng.

Các nhu cầu đầu tiên (parent + propagation + LCA entry/exit) là trọng tâm HSM v2.
