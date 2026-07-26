.syntax unified
.cpu cortex-m3
.thumb

.extern g_hr_current_task_control_block
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

.section .note.GNU-stack,"",%progbits
