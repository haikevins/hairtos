# Architecture port contract

Kernel generic cần một số mechanism mà C portable không cung cấp.

## Required capability

Port phải định nghĩa trong `hr_port_config.h`:

```text
HR_PORT_NAME
HR_PORT_MIN_TASK_STACK_WORDS
HR_PORT_STACK_ALIGNMENT_BYTES
HR_PORT_SUPPORTS_FPU_CONTEXT
HR_PORT_SUPPORTS_MPU
```

Config check dựa vào các capability này.

## Required mechanism

Port hiện cung cấp các nhóm chức năng:

- prepare initial task stack;
- start first task;
- request deferred context switch;
- critical enter/exit;
- ISR detection;
- wait-for-interrupt;
- context switch assembly;
- architecture fault capture;
- tick IRQ adapter;
- optional benchmark clock backend.

## Saved-SP ABI

Internal TCB và assembly phải thống nhất nơi lưu stack pointer. Đây là ABI nội bộ quan trọng nhất giữa C scheduler và assembly.

## Critical section

Port phải preserve prior interrupt state khi enter/exit nested contexts phù hợp. Không chỉ "enable interrupts" vô điều kiện khi exit.

## ISR detection

Kernel task-only API dùng port detection để reject call từ handler mode.

## First task

Port start phải chuyển từ bootstrap/main stack sang task stack đúng ABI và không return khi thành công.

## Deferred switch

Request switch phải an toàn từ task context và ISR context theo architecture.

## FPU/MPU

Không khai capability = 1 chỉ vì CPU có peripheral. FPU context support chỉ đúng nếu port save/restore đầy đủ register state theo exception model.

## Port validation ladder

1. compile initial stack test;
2. first-task startup;
3. cooperative switch;
4. preemption;
5. ISR wake;
6. fault capture;
7. stress;
8. benchmark clock.
