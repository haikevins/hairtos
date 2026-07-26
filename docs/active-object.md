# Active Object Specification

An Active Object owns a HairRTOS task, event queue, flat state machine, stack,
and private context. It is created statically with `he_active_create_static()`.

The internal task loop is:

```text
start state machine
      ↓
block on event queue
      ↓
dispatch one event run-to-completion
      ↓
release dynamic event reference
      ↓
block again
```

Posting APIs:

- `he_active_post()` from task context;
- `he_active_post_from_isr()` from ISR context.

State handlers must not contain unbounded loops, busy waits, or long blocking
operations. Long operations should be split into request and completion events.
Higher-priority HairRTOS tasks may preempt an Active Object, but a single Active
Object never dispatches two events concurrently.
