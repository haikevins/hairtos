
#include "board.h"
#include "stm32f1.h"

uint32_t board_get_core_clock_hz(void)
{
    return SystemCoreClock;
}

bool board_clock_is_nominal(void)
{
    return SystemCoreClock == 72000000UL;
}
