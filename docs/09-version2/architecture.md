# Kiến trúc dự kiến cho Version 2

## Tổng thể

```text
Application
  |
  +--> haievent v2
  |      +-- Event ownership
  |      +-- HSM
  |      +-- Active Object
  |      +-- Deferred event
  |      +-- Time Event / PubSub
  |
  +--> hairtos public API
           |
           +-- task/scheduler/time
           +-- IPC/synchronization
           +-- diagnostics/trace
           |
        kernel internals
           |
        port contract v2
           |
   target capabilities/bindings
```

## Port contract v2

Tách capability và mechanism rõ hơn:

```text
context
interrupt ceiling
tick source
low-power sleep
fault capture
cycle clock
optional FPU
optional MPU
```

Không buộc mọi architecture phải có SysTick/PendSV; generic kernel chỉ cần semantics tương đương.

## Interrupt levels

Trên Cortex-M3/M4, dự kiến:

```text
high urgency ISR
  - không gọi kernel
  - có thể chạy khi kernel critical

kernel-aware ISR
  - priority trong syscall-safe range
  - được gọi *_from_isr

PendSV
  - lowest
```

BASEPRI phù hợp hơn PRIMASK cho mô hình này.

## Time

Generic time giữ tick semantics cho compatibility. Tickless port nhận next deadline và lập wake source.

```text
timeout/timer next deadline
 -> port sleep_until(deadline)
 -> wake
 -> compute elapsed ticks
 -> advance kernel time
```

Cần thiết kế cẩn thận để không phá current timeout ordering.

## haievent HSM

FSM object cần mở rộng state representation. Không nên dùng chỉ function pointer nếu parent metadata cần truy xuất efficient.

Một hướng:

```c
typedef struct he_state
{
    he_state_handler_t handler;
    const struct he_state *parent;
} he_state_t;
```

Nhưng đây là proposed design, chưa final API.

Transition HSM cần xác định least common ancestor, EXIT chain và ENTRY chain.

## Trace

Trace là optional static ring, event records nhỏ. Không dùng printf trong kernel hot path.

```text
kernel event -> trace record
debug/export later
```

## Compatibility

Dedicated-task AO và flat API có thể được implement như subset trên HSM engine để giảm hai engine song song.
