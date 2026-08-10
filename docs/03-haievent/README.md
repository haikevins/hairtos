# 03 — haievent: Event-Driven Framework

`haievent` là framework phía trên `hairtos`, không phải scheduler thứ hai.

```text
Active Object
 = RTOS task
 + event queue
 + state machine
 + user context
```

## Tài liệu

- [architecture.md](architecture.md)
- [event-model.md](event-model.md)
- [ownership-and-rtc.md](ownership-and-rtc.md)
- [state-machine.md](state-machine.md)
- [active-object.md](active-object.md)
- [time-event.md](time-event.md)
- [publish-subscribe.md](publish-subscribe.md)

## Runtime flow

```text
event source
  -> post
  -> AO queue
  -> AO task wakes
  -> dispatch FSM
  -> handler returns
  -> dynamic event release
```

Framework chỉ dùng public kernel API. `haievent` không include `kernel/internal`.
