
# Blue Pill STM32F103C8T6 target manifest.
#
# This file is the only build description that binds hairtos to this MCU and
# board. A new target should provide the same variables in a new manifest.

set(HAIRTOS_TARGET_MANIFEST_VERSION 1)
set(HAIRTOS_TARGET_NAME "bluepill_f103c8")
set(HAIRTOS_TARGET_DESCRIPTION "Blue Pill STM32F103C8T6 / ARM Cortex-M3")
set(HAIRTOS_TARGET_ARCH "arm/cortex-m3")
set(HAIRTOS_TARGET_SOC "stm32f1")
set(HAIRTOS_TARGET_BOARD "bluepill_f103c8")

set(HAIRTOS_TARGET_CPU_FLAGS
    -mcpu=cortex-m3
    -mthumb)

set(HAIRTOS_TARGET_COMPILE_DEFINITIONS
    STM32F103xB)

set(HAIRTOS_TARGET_PUBLIC_INCLUDES
    soc/stm32f1/include
    boards/bluepill_f103c8/include
    drivers/stm32f1/include
    arch/arm/cortex-m3/include)

set(HAIRTOS_TARGET_PLATFORM_C
    soc/stm32f1/system_stm32f1.c
    soc/stm32f1/stm32f1_clock.c
    soc/stm32f1/stm32f1_irq.c
    boards/bluepill_f103c8/board.c
    boards/bluepill_f103c8/board_clock.c
    drivers/stm32f1/hr_gpio_stm32f1.c
    drivers/stm32f1/hr_uart_stm32f1.c
    drivers/stm32f1/hr_hw_timer_stm32f1.c)

set(HAIRTOS_TARGET_PLATFORM_ASM
    soc/stm32f1/startup_stm32f103.S)

set(HAIRTOS_TARGET_PORT_C
    arch/arm/cortex-m3/hr_port.c
    arch/arm/cortex-m3/hr_port_stack.c)
set(HAIRTOS_TARGET_PORT_STACK_C
    arch/arm/cortex-m3/hr_port_stack.c)
set(HAIRTOS_TARGET_PORT_ASM
    arch/arm/cortex-m3/hr_portasm.S)
set(HAIRTOS_TARGET_KERNEL_TICK_C
    arch/arm/cortex-m3/hr_kernel_tick_irq.c)
set(HAIRTOS_TARGET_BAREMETAL_TICK_C
    arch/arm/cortex-m3/hr_baremetal_tick_irq.c)
set(HAIRTOS_TARGET_FAULT_C
    arch/arm/cortex-m3/hr_fault.c)
set(HAIRTOS_TARGET_FAULT_ASM
    arch/arm/cortex-m3/hr_faultasm.S)
set(HAIRTOS_TARGET_BENCHMARK_CLOCK_C
    arch/arm/cortex-m3/hr_benchmark_clock_dwt.c)

set(HAIRTOS_TARGET_LINKER_SCRIPT
    boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld)
set(HAIRTOS_TARGET_OPENOCD_CONFIG
    tools/openocd/bluepill_stlink.cfg)
set(HAIRTOS_TARGET_OPENOCD_ERASE_COMMAND
    "stm32f1x mass_erase 0")
