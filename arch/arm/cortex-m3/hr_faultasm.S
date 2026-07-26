.syntax unified
.cpu cortex-m3
.thumb

.extern hr_port_fault_capture

.macro HR_FAULT_HANDLER handler, reason
.global \handler
.type \handler, %function
.thumb_func
\handler:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    mov r1, lr
    movs r2, #\reason
    b hr_port_fault_capture
.size \handler, . - \handler
.endm

HR_FAULT_HANDLER NMI_Handler,        4
HR_FAULT_HANDLER HardFault_Handler,  5
HR_FAULT_HANDLER MemManage_Handler,  6
HR_FAULT_HANDLER BusFault_Handler,   7
HR_FAULT_HANDLER UsageFault_Handler, 8

.section .note.GNU-stack,"",%progbits
