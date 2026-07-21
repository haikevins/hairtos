#include "stm32f1.h"

static void fault_stop(void)
{
    stm32f1_cpu_disable_irq();

    for (;;)
    {
        stm32f1_cpu_breakpoint();
    }
}

void HardFault_Handler(void)
{
    fault_stop();
}

void MemManage_Handler(void)
{
    fault_stop();
}

void BusFault_Handler(void)
{
    fault_stop();
}

void UsageFault_Handler(void)
{
    fault_stop();
}
