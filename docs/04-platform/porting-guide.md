# Porting guide — tư duy

## Hai loại port

### Board mới, cùng SoC/CPU

Thường reuse:

```text
arch
soc
drivers
```

Thay board pins/services/linker/debug manifest.

### SoC mới, cùng CPU architecture

Reuse architecture port + kernel/framework; viết SoC/startup/driver/board.

### CPU architecture mới

Ngoài platform, phải viết context/stack/critical/ISR/fault mechanism mới.

## Không sửa kernel để port

Nếu cần thêm `#ifdef NEW_MCU` vào scheduler/queue/mutex, đó là dấu hiệu boundary đang sai.

## Port order

1. startup + linker + bare-metal UART/LED;
2. bare-metal tick;
3. initial stack;
4. first task;
5. cooperative switch;
6. priority/preemption;
7. timeout;
8. IPC;
9. timer;
10. haievent;
11. benchmark;
12. diagnostics/fault.

## Capability audit

Trước khi reuse port, kiểm tra:

- register width;
- stack alignment;
- callee-saved set;
- exception frame;
- interrupt masking;
- ISR nesting;
- atomicity;
- FPU/lazy stacking;
- MPU;
- low-power wake/tick source.

## Definition of done

Port không "xong" chỉ vì build. Cần hardware logs + stress + fault + memory/benchmark validation.
