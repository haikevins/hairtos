# Synchronization Model

Queues copy fixed-size items into static storage and support blocking/nonblocking
send and receive, finite timeout, wait forever, and ISR-safe nonblocking paths.
Waiters are ordered by highest priority, FIFO among equal priorities.

Semaphores have no owner. Binary semaphores represent an event; counting
semaphores represent resources or accumulated events.

Mutexes are task-context-only and have an owner. v1.0 targets non-recursive
mutexes, timeout, owner validation, waiter priority ordering, and basic priority
inheritance.

Priority restoration considers all mutexes still owned by the task; releasing
one mutex must not blindly restore base priority.

Application design should prefer message passing and ownership transfer before
shared mutable state.
