cmake_minimum_required(VERSION 3.20)
project(HairRTOS LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(HAIRTOS_EXAMPLE "01-baremetal-foundation" CACHE STRING "Implemented target example")
set(HAIRTOS_TARGET_EXAMPLES
    01-baremetal-foundation
    03-static-task-stack
    04-start-first-task
    05-cooperative-context-switch
)

list(FIND HAIRTOS_TARGET_EXAMPLES "${HAIRTOS_EXAMPLE}" HAIRTOS_EXAMPLE_INDEX)
if(HAIRTOS_EXAMPLE_INDEX EQUAL -1)
    message(FATAL_ERROR
        "${HAIRTOS_EXAMPLE} is not an implemented STM32 target example. "
        "Run the Phase 2 host demo with 'make phase2-example'.")
endif()

set(HAIRTOS_KERNEL_SOURCES "")
set(HAIRTOS_PORT_ASM "")
if(HAIRTOS_EXAMPLE STREQUAL "03-static-task-stack" OR
   HAIRTOS_EXAMPLE STREQUAL "04-start-first-task" OR
   HAIRTOS_EXAMPLE STREQUAL "05-cooperative-context-switch")
    list(APPEND HAIRTOS_KERNEL_SOURCES
        kernel/src/hr_list.c
        kernel/src/hr_scheduler.c
        kernel/src/hr_wait.c
        kernel/src/hr_timeout.c
        kernel/src/hr_task.c
        arch/arm/cortex-m3/hr_port.c
        arch/arm/cortex-m3/hr_port_stack.c
    )
endif()

if(HAIRTOS_EXAMPLE STREQUAL "04-start-first-task" OR
   HAIRTOS_EXAMPLE STREQUAL "05-cooperative-context-switch")
    list(APPEND HAIRTOS_KERNEL_SOURCES kernel/src/hr_kernel.c)
    list(APPEND HAIRTOS_PORT_ASM arch/arm/cortex-m3/hr_portasm.S)
endif()

add_executable(hairtos_baremetal
    soc/stm32f1/startup_stm32f103.S
    soc/stm32f1/system_stm32f1.c
    soc/stm32f1/stm32f1_clock.c
    soc/stm32f1/stm32f1_irq.c
    boards/bluepill_f103c8/board.c
    boards/bluepill_f103c8/board_clock.c
    drivers/gpio/src/hr_gpio_stm32f1.c
    drivers/uart/src/hr_uart_stm32f1.c
    drivers/timer/src/hr_hw_timer_stm32f1.c
    examples/${HAIRTOS_EXAMPLE}/main.c
    ${HAIRTOS_KERNEL_SOURCES}
    ${HAIRTOS_PORT_ASM}
)

target_include_directories(hairtos_baremetal PRIVATE
    config
    soc/stm32f1/include
    boards/bluepill_f103c8/include
    drivers/gpio/include
    drivers/uart/include
    drivers/timer/include
    kernel/include
    kernel/internal
    arch/arm/cortex-m3/include
)

target_compile_options(hairtos_baremetal PRIVATE
    -mcpu=cortex-m3 -mthumb -Og -g3 -ffreestanding -fno-common
    -fno-builtin -ffunction-sections -fdata-sections
    -Wall -Wextra -Werror -Wshadow -Wundef -Wconversion -Wsign-conversion
)

target_link_options(hairtos_baremetal PRIVATE
    -mcpu=cortex-m3 -mthumb -nostdlib -nostartfiles
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/hairtos_baremetal.map
    -T${CMAKE_CURRENT_SOURCE_DIR}/boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld
)

target_link_libraries(hairtos_baremetal PRIVATE gcc)
set_target_properties(hairtos_baremetal PROPERTIES SUFFIX ".elf")
