# Flat State Machine Specification

A handler returns one of:

- `HE_STATE_HANDLED`;
- `HE_STATE_IGNORED`;
- `HE_STATE_TRANSITION`.

A transition is requested with `he_state_transition(machine, target)`.
HairEvent applies the transition in this order:

```text
EXIT current state
change current-state pointer
ENTRY target state
INIT target state
```

`ENTRY`, `EXIT`, and `INIT` are framework-generated immutable events. Normal
application events are dispatched run-to-completion by the owning Active Object.

Phase 13 is intentionally flat. Hierarchical state ancestry, bubbling, and
least-common-ancestor exit/entry paths are not implemented in this archive.
