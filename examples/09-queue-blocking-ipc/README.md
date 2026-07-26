# Phase 9 — Queue and Blocking IPC

This STM32 target example demonstrates a statically allocated fixed-size FIFO
queue with blocking producer/consumer behavior.

- Consumer priority: 1
- Producer priority: 3
- Queue capacity: two `phase9_message_t` items
- Consumer receive timeout: `HR_WAIT_FOREVER`
- Producer send timeout: 100 ticks
- Consumer processing delay: 200 ticks

The higher-priority consumer starts first and blocks on an empty queue. The
producer directly hands the first item to the blocked consumer, which becomes
READY and preempts the producer. While the consumer is delayed, the producer
fills the queue and eventually blocks or times out.

Build:

```bash
make EXAMPLE=09-queue-blocking-ipc
```

Flash:

```bash
make EXAMPLE=09-queue-blocking-ipc flash
```

Expected UART pattern:

```text
HairRTOS Phase 9
Static FIFO queue with blocking send/receive and timeout.
...
consumer received seq=1 produced_at=0 now=0 queued=0 send_timeouts=0
consumer received seq=2 produced_at=0 now=200 queued=1 send_timeouts=1
...
```

Exact sequence gaps depend on send timeouts. Received sequence numbers must
always increase, proving FIFO order for successfully queued messages.
