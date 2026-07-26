.syntax unified
.cpu cortex-m3
.thumb

.extern g_hr_current_task_control_block
.extern hr_kernel_select_next_from_pendsv
.extern _estack

.global hr_port_start_first_task
.type hr_port_start_first_task, %function
.thumb_func
hr_port_start_first_task:
    cpsid i
    ldr r0, =_estack
    msr msp, r0
    dsb
    isb
    cpsie i
    svc #0
1:
    b 1b
.size hr_port_start_first_task, . - hr_port_start_first_task

.global SVC_Handler
.type SVC_Handler, %function
.thumb_func
SVC_Handler:
    ldr r0, =g_hr_current_task_control_block
    ldr r0, [r0]
    cbz r0, 2f

    /* stack_pointer is the first field of the internal TCB. */
    ldr r0, [r0]
    ldmia r0!, {r4-r11}
    msr psp, r0

    /* Privileged Thread mode, PSP selected. */
    movs r0, #2
    msr control, r0
    isb

    ldr lr, =0xFFFFFFFD
    bx lr

2:
    bkpt #0
    b 2b
.size SVC_Handler, . - SVC_Handler

.global PendSV_Handler
.type PendSV_Handler, %function
.thumb_func
PendSV_Handler:
    /* Hardware has already stacked R0-R3, R12, LR, PC, and xPSR on PSP. */
    mrs r0, psp
    ldr r3, =g_hr_current_task_control_block
    ldr r2, [r3]
    cbz r2, 3f

    /* Save the software context and publish the new saved PSP in the TCB. */
    stmdb r0!, {r4-r11}
    str r0, [r2]

    /* Keep MSP 8-byte aligned across the C call and preserve EXC_RETURN. */
    push {r3, lr}
    bl hr_kernel_select_next_from_pendsv
    pop {r3, lr}

    /* The C selector updates g_hr_current_task_control_block. */
    ldr r2, [r3]
    cbz r2, 3f
    ldr r0, [r2]
    ldmia r0!, {r4-r11}
    msr psp, r0
    bx lr

3:
    bkpt #0
    b 3b
.size PendSV_Handler, . - PendSV_Handler

.section .note.GNU-stack,"",%progbits
