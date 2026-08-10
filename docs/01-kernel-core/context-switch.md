# Context switch

## Generic contract

Kernel không biết register set. Port cung cấp:

- initial stack;
- start first task;
- request context switch;
- critical section;
- ISR detection;
- wait-for-interrupt.

## Cortex-M3 initial frame

Hardware exception frame:

```text
R0 R1 R2 R3 R12 LR PC xPSR
```

Software-saved frame:

```text
R4 R5 R6 R7 R8 R9 R10 R11
```

R0 = task argument, PC = entry, xPSR Thumb bit set.

## First task

```text
main/MSP
  -> hr_kernel_start
  -> SVC
  -> restore software frame
  -> PSP = task stack
  -> Thread mode uses PSP
  -> exception return
  -> task entry
```

## PendSV

```text
PendSV entry
  -> read PSP
  -> save R4-R11
  -> store saved SP in current TCB
  -> call C selector
  -> load next TCB saved SP
  -> restore R4-R11
  -> write PSP
  -> exception return
```

Handler mode dùng MSP; task Thread mode dùng PSP.

## Atomicity

Selector chạy với interrupt state phù hợp để internal ready/wait structures không bị ISR sửa giữa save/select/restore.

## FPU

Port v1 không lưu FPU context. `HR_PORT_SUPPORTS_FPU_CONTEXT=0`; config check cấm bật FPU feature.

## ABI invariant

Assembly phụ thuộc saved stack pointer field offset. Mọi refactor TCB phải giữ static assertion/contract đồng bộ với assembly.

## Validation

- host test initial stack layout;
- example 04 first task;
- example 05 local variable preservation;
- example 08 preemption/time slice.
