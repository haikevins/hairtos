# Cortex-M3 port

## 1. Mục tiêu

Cung cấp primitives architecture-specific mà kernel generic cần: initial stack, critical section, SVC/PendSV, SysTick integration, ISR detection và fault capture.

## 2. File chính

```text
arch/arm/cortex-m3/hr_port.c
arch/arm/cortex-m3/hr_port_stack.c
arch/arm/cortex-m3/hr_portasm.S
arch/arm/cortex-m3/hr_fault.c
arch/arm/cortex-m3/hr_faultasm.S
```

## 3. Port C API

Port cấu hình exception priority, đọc/ghi PRIMASK/CONTROL/PSP, pend PendSV, kiểm tra IPSR và thực hiện WFI.

## 4. Assembly ABI

- TCB field 0 phải là saved SP.
- SVC restore initial software frame và exception-return bằng PSP.
- PendSV save/restore R4–R11.
- Fault wrapper chọn MSP hoặc PSP dựa trên EXC_RETURN rồi chuyển frame sang C.

## 5. Stack alignment

Initial SP căn chỉnh 8 byte theo ARM EABI/exception requirements.

## 6. Fault record

Port đọc SCB fault registers và ghi `hr_fault_context_t`; retained record nằm trong `.noinit`.

## 7. Kiểm thử

Validation kiểm tra symbol strength, disassembly pattern và compile proof một phần cho Cortex-M0 generic structures.

## 8. Giới hạn

Không FPU, MPU, TrustZone hoặc low-power tickless idle.
