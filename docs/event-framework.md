# HairEvent Framework

HairEvent adds Event-Driven Programming above HairRTOS without replacing the
preemptive kernel.

An Active Object is one HairRTOS task, one event queue, one current state
handler, and private state. Its task waits for one event, dispatches it, releases
it when necessary, and blocks again.

Handlers are run-to-completion: no busy waits, unbounded loops, long mutex holds,
or lengthy blocking calls. Higher-priority RTOS tasks may still preempt them.

Reserved signals include NONE, ENTRY, EXIT, INIT, and TIMEOUT. Application
signals begin at `HE_SIG_USER`. The kernel never interprets signal values.

Static events are immutable and never freed. For a direct dynamic post, a
successful queue insertion transfers ownership to the receiver/framework. A
failed post leaves ownership with the sender.

Time events post TIMEOUT into the Active Object queue; SysTick and timer ISR
contexts never call state handlers directly.

Publish/subscribe and hierarchical state machines are deferred until direct
posting, ownership, reference counting, and flat transitions are proven.
