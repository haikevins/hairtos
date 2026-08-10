# Time Event

## Mục đích

Biến software timer expiration thành event tới AO mà không chạy state handler trong timer-service context.

## Composition

Time Event chứa:

- kernel timer;
- target AO;
- embedded static event;
- signal;
- dropped counter;
- validity/armed metadata.

## Flow

```text
kernel tick
 -> software timer expires
 -> timer-service callback
 -> nonblocking post static event to target AO
 -> target AO dispatch
```

## Dropped counter

Nếu AO queue không nhận event, time event tăng dropped count. Điều này giúp phát hiện queue capacity/priority không phù hợp.

## Commands

Create, arm, disarm, rearm, change period.

## Lifetime

Embedded event là static và được tái sử dụng. Handler không được giữ pointer để dùng asynchronous sau dispatch.

## V2

Trace nên record expiration, post success/drop và dispatch latency. HSM timeouts có thể build trên cùng primitive thay vì tạo timer semantics khác.
