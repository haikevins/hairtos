# Active Object Specification

An Active Object owns a task, event queue, current state, priority, event
storage, and private application data.

Only its backing task writes private state. Other modules communicate by posting
events.

The internal loop may block on its event queue. State handlers should not block;
long operations are split into start/completion events.

ISRs may post static or preallocated events through ISR-safe APIs, but never
dispatch a state machine directly.
