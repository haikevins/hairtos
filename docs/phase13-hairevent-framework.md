# Phase 13 — HairEvent Framework

## Implemented capabilities

1. static events and user-defined payload structures;
2. fixed-block dynamic event pools;
3. reference retain/release and deterministic pool return;
4. direct task-context event posting;
5. ISR-safe non-blocking event posting;
6. statically created Active Objects;
7. flat state machines with ENTRY, EXIT, INIT, and transitions;
8. software-timer-backed one-shot and periodic time events;
9. fixed-capacity publish/subscribe;
10. six focused STM32 examples and host sanitizer coverage.

## Ownership rules

| Operation | Dynamic-event ownership |
|---|---|
| `he_event_new()` | Caller owns one reference |
| Successful `he_active_post()` | Reference transfers to receiver |
| Failed `he_active_post()` | Caller keeps reference |
| `he_pubsub_publish()` | One retained reference per successful subscriber; publisher reference is consumed |
| Active Object after dispatch | Releases its reference |
| Static event | Never freed and retain/release are no-ops |

## Deterministic memory

HairEvent does not use `malloc`. Applications provide:

- fixed-size event-pool block arrays;
- Active Object queue pointer arrays;
- Active Object stacks;
- publish/subscribe subscriber matrices.

## Definition-of-Done demonstration

`13-01-event-post` uses EXTI0 software interrupt to post a static event. The
Active Object wakes after exception return, dispatches the event in task context,
and blocks again. The remaining examples prove Active Objects, flat transitions,
time events, dynamic multicast ownership, and integrated operation.

## Not included

- hierarchical state machines;
- event deferral/recall;
- dynamic subscription storage;
- heap-backed events;
- time-event APIs callable directly from ISR.
