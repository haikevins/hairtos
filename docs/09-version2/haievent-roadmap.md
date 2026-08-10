# haievent roadmap Version 2

## H1 — Hierarchical State Machine

Đây là feature chính.

### Cần hỗ trợ tối thiểu

- state có parent;
- event không handled có thể propagate lên parent;
- transition source/target khác level;
- EXIT chain đến LCA;
- ENTRY chain từ LCA xuống target;
- initial substate;
- bounded transition chain.

### Ví dụ

```text
Operational
├── Idle
└── Active
    ├── Normal
    └── Limited
```

`STOP` có thể được handle ở `Operational` thay vì duplicate ở `Idle/Active/Normal/Limited`.

### Test cần có

- self transition;
- sibling;
- child→parent;
- parent→child;
- cross-branch;
- inherited event;
- initial chain;
- invalid parent cycle;
- max depth.

## H2 — Flat compatibility

Flat states phải là HSM depth=1 hoặc compatibility wrapper, tránh duy trì hai engines có semantics lệch.

## H3 — Deferred events

State có thể defer event chưa xử lý được vào caller-provided queue/list, sau đó recall.

Ownership đặc biệt quan trọng:

- dynamic event phải giữ reference khi defer;
- recall chuyển reference trở lại dispatch queue;
- reset/state exit policy phải rõ.

## H4 — RTC enforcement

Framework set dispatch-context flag trước state handler. Debug build kernel blocking APIs có thể detect và assert nếu caller đang trong AO RTC context.

Cần tránh coupling ngược `kernel -> haievent`; một cách là generic "blocking forbidden context" hook/token trong context API, không hard-code AO type.

## H5 — AO error policy

Invalid event/FSM internal error -> diagnostics hook/panic, không yield-spin vô hạn.

## H6 — Observability

Trace:

- post source/target;
- queue depth;
- dispatch signal;
- transition source/target;
- time-event drop;
- pubsub delivered count;
- pool allocation/free.

## H7 — Shared executor (stretch)

Dedicated task/AO vẫn mặc định.

Experimental shared executor có thể cho nhiều low-rate AO dùng một worker để giảm stack RAM, nhưng cần policy priority/fairness rõ. Không đưa vào 2.0 critical path nếu làm chậm HSM/portability.
