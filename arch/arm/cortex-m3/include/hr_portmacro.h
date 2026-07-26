#ifndef HR_PORTMACRO_H
#define HR_PORTMACRO_H

#include <stddef.h>
#include <stdint.h>

#include "hairtos/hr_types.h"

#define HR_PORT_INITIAL_XPSR              0x01000000UL
#define HR_PORT_THUMB_ADDRESS_BIT         0x00000001UL
#define HR_PORT_ADDRESS_MASK              0xFFFFFFFEUL
#define HR_PORT_STACK_ALIGNMENT_BYTES     8U
#define HR_PORT_INITIAL_FRAME_WORDS        16U

#define HR_PORT_FRAME_R4_INDEX             0U
#define HR_PORT_FRAME_R5_INDEX             1U
#define HR_PORT_FRAME_R6_INDEX             2U
#define HR_PORT_FRAME_R7_INDEX             3U
#define HR_PORT_FRAME_R8_INDEX             4U
#define HR_PORT_FRAME_R9_INDEX             5U
#define HR_PORT_FRAME_R10_INDEX            6U
#define HR_PORT_FRAME_R11_INDEX            7U
#define HR_PORT_FRAME_R0_INDEX             8U
#define HR_PORT_FRAME_R1_INDEX             9U
#define HR_PORT_FRAME_R2_INDEX             10U
#define HR_PORT_FRAME_R3_INDEX             11U
#define HR_PORT_FRAME_R12_INDEX            12U
#define HR_PORT_FRAME_LR_INDEX             13U
#define HR_PORT_FRAME_PC_INDEX             14U
#define HR_PORT_FRAME_XPSR_INDEX           15U

hr_stack_t *hr_port_initialize_stack_raw(hr_stack_t *stack_low,
                                         size_t stack_words,
                                         uint32_t entry_address,
                                         uint32_t argument_address,
                                         uint32_t exit_address);

#endif /* HR_PORTMACRO_H */
