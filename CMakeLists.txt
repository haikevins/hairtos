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
    06-priority-scheduler
    07-task-delay-timeout
    08-preemption-round-robin
    09-queue-blocking-ipc
    10-01-semaphore-from-isr
    10-02-mutex-priority-inheritance
    11-task-suspend-resume
    12-software-timer
    13-01-event-post
    13-02-active-object
    13-03-flat-state-machine
    13-04-time-event
    13-05-publish-subscribe
    13-06-event-driven-demo
    14-memory-allocator-lab
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
   HAIRTOS_EXAMPLE STREQUAL "05-cooperative-context-switch" OR
   HAIRTOS_EXAMPLE STREQUAL "06-priority-scheduler" OR
   HAIRTOS_EXAMPLE STREQUAL "07-task-delay-timeout" OR
   HAIRTOS_EXAMPLE STREQUAL "08-preemption-round-robin" OR
   HAIRTOS_EXAMPLE STREQUAL "09-queue-blocking-ipc" OR
   HAIRTOS_EXAMPLE STREQUAL "10-01-semaphore-from-isr" OR
   HAIRTOS_EXAMPLE STREQUAL "10-02-mutex-priority-inheritance" OR
   HAIRTOS_EXAMPLE STREQUAL "11-task-suspend-resume" OR
   HAIRTOS_EXAMPLE STREQUAL "12-software-timer" OR
   HAIRTOS_EXAMPLE MATCHES "^13-")
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
   HAIRTOS_EXAMPLE STREQUAL "05-cooperative-context-switch" OR
   HAIRTOS_EXAMPLE STREQUAL "06-priority-scheduler" OR
   HAIRTOS_EXAMPLE STREQUAL "07-task-delay-timeout" OR
   HAIRTOS_EXAMPLE STREQUAL "08-preemption-round-robin" OR
   HAIRTOS_EXAMPLE STREQUAL "09-queue-blocking-ipc" OR
   HAIRTOS_EXAMPLE STREQUAL "10-01-semaphore-from-isr" OR
   HAIRTOS_EXAMPLE STREQUAL "10-02-mutex-priority-inheritance" OR
   HAIRTOS_EXAMPLE STREQUAL "11-task-suspend-resume" OR
   HAIRTOS_EXAMPLE STREQUAL "12-software-timer" OR
   HAIRTOS_EXAMPLE MATCHES "^13-")
    list(APPEND HAIRTOS_KERNEL_SOURCES kernel/src/hr_kernel.c)
    list(APPEND HAIRTOS_PORT_ASM arch/arm/cortex-m3/hr_portasm.S)
endif()

if(HAIRTOS_EXAMPLE STREQUAL "09-queue-blocking-ipc")
    list(APPEND HAIRTOS_KERNEL_SOURCES kernel/src/hr_queue.c)
endif()

if(HAIRTOS_EXAMPLE STREQUAL "10-01-semaphore-from-isr")
    list(APPEND HAIRTOS_KERNEL_SOURCES kernel/src/hr_semaphore.c)
endif()

if(HAIRTOS_EXAMPLE STREQUAL "10-02-mutex-priority-inheritance")
    list(APPEND HAIRTOS_KERNEL_SOURCES kernel/src/hr_mutex.c)
endif()

if(HAIRTOS_EXAMPLE STREQUAL "12-software-timer")
    list(APPEND HAIRTOS_KERNEL_SOURCES
        kernel/src/hr_semaphore.c
        kernel/src/hr_timer.c
    )
endif()

if(HAIRTOS_EXAMPLE MATCHES "^13-")
    list(APPEND HAIRTOS_KERNEL_SOURCES
        kernel/src/hr_context.c
        kernel/src/hr_queue.c
        kernel/src/hr_semaphore.c
        kernel/src/hr_timer.c
        hairevent/src/he_event.c
        hairevent/src/he_state_machine.c
        hairevent/src/he_active.c
        hairevent/src/he_time_event.c
        hairevent/src/he_pubsub.c
    )
endif()

set(HAIRTOS_LAB_SOURCES "")
if(HAIRTOS_EXAMPLE STREQUAL "14-memory-allocator-lab")
    list(APPEND HAIRTOS_LAB_SOURCES
        labs/memory-allocator/src/hr_heap_lab.c
        labs/memory-allocator/src/hr_pool_lab.c
    )
endif()

set(HAIRTOS_TICK_SOURCE drivers/timer/src/hr_systick_baremetal_irq.c)
if(HAIRTOS_EXAMPLE STREQUAL "07-task-delay-timeout" OR
   HAIRTOS_EXAMPLE STREQUAL "08-preemption-round-robin" OR
   HAIRTOS_EXAMPLE STREQUAL "09-queue-blocking-ipc" OR
   HAIRTOS_EXAMPLE STREQUAL "10-01-semaphore-from-isr" OR
   HAIRTOS_EXAMPLE STREQUAL "10-02-mutex-priority-inheritance" OR
   HAIRTOS_EXAMPLE STREQUAL "11-task-suspend-resume" OR
   HAIRTOS_EXAMPLE STREQUAL "12-software-timer" OR
   HAIRTOS_EXAMPLE MATCHES "^13-")
    set(HAIRTOS_TICK_SOURCE kernel/src/hr_time.c)
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
    ${HAIRTOS_TICK_SOURCE}
    examples/${HAIRTOS_EXAMPLE}/main.c
    ${HAIRTOS_KERNEL_SOURCES}
    ${HAIRTOS_LAB_SOURCES}
    ${HAIRTOS_PORT_ASM}
)

if(HAIRTOS_EXAMPLE STREQUAL "07-task-delay-timeout")
    target_compile_definitions(hairtos_baremetal PRIVATE
        HR_CFG_PREEMPTION=0
        HR_CFG_TIME_SLICING=0
    )
endif()

if(HAIRTOS_EXAMPLE STREQUAL "12-software-timer" OR HAIRTOS_EXAMPLE MATCHES "^13-")
    target_compile_definitions(hairtos_baremetal PRIVATE
        HR_CFG_ENABLE_SOFTWARE_TIMER=1
        HR_CFG_TIMER_TASK_PRIORITY=1
    )
else()
    target_compile_definitions(hairtos_baremetal PRIVATE
        HR_CFG_ENABLE_SOFTWARE_TIMER=0
    )
endif()

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
    hairevent/include
    hairevent/internal
    labs/memory-allocator/include
)

target_compile_options(hairtos_baremetal PRIVATE
    -mcpu=cortex-m3 -mthumb -Og -g3 -ffreestanding -fno-common
    -fno-builtin -ffunction-sections -fdata-sections
    -Wall -Wextra -Werror -Wshadow -Wundef -Wconversion -Wsign-conversion
)

target_link_options(hairtos_baremetal PRIVATE
    -mcpu=cortex-m3 -mthumb -nostdlib
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/hairtos_baremetal.map
    -T${CMAKE_CURRENT_SOURCE_DIR}/boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld
)

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_link_libraries(hairtos_baremetal PRIVATE gcc)
endif()
set_target_properties(hairtos_baremetal PROPERTIES SUFFIX ".elf")
