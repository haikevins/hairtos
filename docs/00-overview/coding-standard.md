# Coding standard

## Ngôn ngữ

- C11 cho C.
- GNU assembler-with-cpp cho Cortex-M assembly.
- Freestanding target.
- Không phụ thuộc undefined behavior có chủ đích.

## Naming

```text
hr_*       public hairtos
he_*       public haievent
HR_CFG_*   kernel config
HE_CFG_*   framework config
HR_PORT_*  port capability
board_*    board service
```

Internal symbol vẫn dùng `hr_`/`he_`, nhưng header nằm trong `internal/`.

## Memory

- Không VLA trong kernel hot path.
- Không recursion không giới hạn.
- Không `malloc` trong kernel.
- Mọi size multiplication phải xem overflow.
- Opaque storage phải align đủ cho internal object.

## Concurrency

- Ready/wait/timeout membership update phải atomic.
- Critical section ngắn.
- Không UART print trong critical section.
- ISR API không block.
- Application callback không chạy từ tick ISR.
- Mutex không dùng trong ISR.

## State machine

State handler nên:

- không block;
- không delay;
- không giữ mutex lâu;
- không dispatch reentrant cùng FSM;
- hoàn thành một event nhanh rồi return.

v1 chưa enforce hoàn toàn rule này; đây là contract cần tuân thủ.

## Error handling

Public API trả `hr_status_t`. Invalid argument khác invalid state. Corruption nội bộ nên dẫn đến diagnostics/assert/panic thay vì tiếp tục trong state không xác định.

## Header

- include guard duy nhất;
- public header không expose internal control block;
- source include header của chính module;
- target-specific header không lan lên generic layer.

## Build hygiene

Warning được coi là error trong target/host flags. Feature mới phải tham gia source mapping chính thức; không giữ placeholder `.c/.h` rỗng.
