# Phase 11 — Suspend and Resume

Phase 11 implements administrative task suspension without introducing another
scheduler container. A suspended task is absent from the ready set. If it was
blocked, its existing object-wait and timeout nodes remain linked until the wait
completes or the task is resumed back to BLOCKED.

Implemented behavior:

- suspend READY task;
- self-suspend RUNNING task;
- suspend BLOCKED delay/object waiter;
- preserve finite timeout and wait-object state;
- keep completed waits suspended until explicit resume;
- preempt after resuming a strictly higher-priority task;
- reject idle-task, duplicate, invalid-state, and ISR operations.

`hr_task_resume_from_isr()` is intentionally deferred. Phase 11 exposes only
task-context administrative control.
