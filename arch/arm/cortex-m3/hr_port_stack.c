#include <stdint.h>

#include "hr_portmacro.h"

static uintptr_t hr_port_align_down(uintptr_t value, uintptr_t alignment)
{
    return value & ~(alignment - 1U);
}

hr_stack_t *hr_port_initialize_stack_raw(hr_stack_t *stack_low,
                                         size_t stack_words,
                                         uint32_t entry_address,
                                         uint32_t argument_address,
                                         uint32_t exit_address)
{
    uintptr_t low_address;
    uintptr_t top_address;
    uintptr_t aligned_top_address;
    size_t available_words;
    hr_stack_t *stack_pointer;

    if ((stack_low == NULL) || (stack_words < HR_PORT_INITIAL_FRAME_WORDS))
    {
        return NULL;
    }

    low_address = (uintptr_t)stack_low;
    top_address = (uintptr_t)&stack_low[stack_words];
    aligned_top_address = hr_port_align_down(top_address,
                                             (uintptr_t)HR_PORT_STACK_ALIGNMENT_BYTES);

    if (aligned_top_address <= low_address)
    {
        return NULL;
    }

    available_words = (size_t)((aligned_top_address - low_address) / sizeof(hr_stack_t));
    if (available_words < HR_PORT_INITIAL_FRAME_WORDS)
    {
        return NULL;
    }

    stack_pointer = (hr_stack_t *)aligned_top_address;
    stack_pointer -= HR_PORT_INITIAL_FRAME_WORDS;

    stack_pointer[HR_PORT_FRAME_R4_INDEX] = 0x44444444UL;
    stack_pointer[HR_PORT_FRAME_R5_INDEX] = 0x55555555UL;
    stack_pointer[HR_PORT_FRAME_R6_INDEX] = 0x66666666UL;
    stack_pointer[HR_PORT_FRAME_R7_INDEX] = 0x77777777UL;
    stack_pointer[HR_PORT_FRAME_R8_INDEX] = 0x88888888UL;
    stack_pointer[HR_PORT_FRAME_R9_INDEX] = 0x99999999UL;
    stack_pointer[HR_PORT_FRAME_R10_INDEX] = 0xAAAAAAAAUL;
    stack_pointer[HR_PORT_FRAME_R11_INDEX] = 0xBBBBBBBBUL;

    stack_pointer[HR_PORT_FRAME_R0_INDEX] = argument_address;
    stack_pointer[HR_PORT_FRAME_R1_INDEX] = 0x11111111UL;
    stack_pointer[HR_PORT_FRAME_R2_INDEX] = 0x22222222UL;
    stack_pointer[HR_PORT_FRAME_R3_INDEX] = 0x33333333UL;
    stack_pointer[HR_PORT_FRAME_R12_INDEX] = 0x12121212UL;
    stack_pointer[HR_PORT_FRAME_LR_INDEX] = exit_address | HR_PORT_THUMB_ADDRESS_BIT;
    stack_pointer[HR_PORT_FRAME_PC_INDEX] = entry_address & HR_PORT_ADDRESS_MASK;
    stack_pointer[HR_PORT_FRAME_XPSR_INDEX] = HR_PORT_INITIAL_XPSR;

    return stack_pointer;
}
