# HairEvent Framework

HairEvent is a static-first Event-Driven Programming layer above HairRTOS. It
uses the completed queue, task, timeout, preemption, and software-timer services;
it does not replace the kernel scheduler.

## Event model

A signal is a 16-bit value. Signals `NONE`, `ENTRY`, `EXIT`, `INIT`, and
`TIMEOUT` are reserved. Application signals begin at `HE_SIG_USER`.

Static events are immutable and have no lifetime management. Dynamic events are
allocated from a user-provided fixed-block pool and carry a reference count.
There is no heap dependency.

For direct posting, a successful post transfers one dynamic-event reference to
the receiving Active Object. A failed post leaves that reference with the
sender. Publish/subscribe retains one reference for every successful subscriber
and consumes the publisher's original reference.

## Active Object model

An Active Object combines:

- one HairRTOS task;
- one fixed-capacity queue of `he_event_t *`;
- one flat state machine;
- one task stack;
- one private application context.

Its task blocks on the queue, dispatches one event run-to-completion, releases
the event, then blocks again. Only the Active Object task should mutate its
private context.

## ISR boundary

ISRs may post a static or already allocated event using
`he_active_post_from_isr()`. They never call state handlers. A woken
higher-priority Active Object is selected through the normal PendSV path after
ISR return.

## Time events

A HairEvent time event wraps a HairRTOS software timer and an immutable static
event. The timer-service callback posts that event to the target Active Object.
The target state handler still runs in the Active Object task context.

## Publish/subscribe

A bus uses caller-supplied subscriber-slot storage. Subscription capacity and
signal range are fixed at initialization, making memory use deterministic.
Publishing snapshots the subscriber list, then posts outside the bus critical
section.

## Scope boundary

Phase 13 implements flat state machines. Hierarchical parent relationships,
event bubbling, least-common-ancestor transition paths, event deferral, and
recall queues are intentionally deferred.
