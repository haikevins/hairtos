# Phase 13.6 — Event-Driven Integration Demo

Combines an Active Object, flat state transitions, periodic time events, a
fixed-block dynamic event pool, publish/subscribe, preemption, and blocking
queues. The controller transitions between IDLE and ACTIVE, publishes heartbeat
status, and an observer consumes the status event.

```bash
make EXAMPLE=13-06-event-driven-demo flash
```
