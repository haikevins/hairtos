# State Machine Specification

Implementation order is flat states, ENTRY/EXIT, initial transition, time events,
then hierarchical states after v1.0.

A handler returns HANDLED, IGNORED, or TRANSITION.

A flat transition sends EXIT to the current state, changes the current state
pointer, then sends ENTRY to the target state.

HSM support later requires parent relationships, event bubbling,
least-common-ancestor computation, and correct ordered exit/entry paths.
