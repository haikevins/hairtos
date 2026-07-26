# Event Post

EXTI0 software interrupt posts an immutable static event through
`he_active_post_from_isr()`. The Active Object wakes, dispatches the event
run-to-completion in task context, then blocks on its queue again.

```bash
make EXAMPLE=13-01-event-post run
```
