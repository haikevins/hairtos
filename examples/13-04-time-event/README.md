# Time Event

A periodic software timer posts a static event to an Active Object. The timer
service never invokes the state handler directly; dispatch occurs in the
Active Object task context.

```bash
make EXAMPLE=13-04-time-event run
```
