#include "hr_benchmark.h"

#include <stdint.h>

#define HR_BENCHMARK_DEMCR_ADDRESS      0xE000EDFCUL
#define HR_BENCHMARK_DWT_CTRL_ADDRESS   0xE0001000UL
#define HR_BENCHMARK_DWT_CYCCNT_ADDRESS 0xE0001004UL
#define HR_BENCHMARK_DEMCR_TRCENA       (UINT32_C(1) << 24U)
#define HR_BENCHMARK_DWT_CYCCNTENA      (UINT32_C(1) << 0U)
#define HR_BENCHMARK_DWT_NOCYCCNT       (UINT32_C(1) << 25U)
#define HR_BENCHMARK_REG32(address) \
    (*(volatile uint32_t *)(uintptr_t)(address))

static uint32_t g_benchmark_core_clock_hz;

bool hr_benchmark_clock_init(uint32_t core_clock_hz)
{
    uint32_t before;
    uint32_t after;
    uint32_t index;

    if (core_clock_hz == 0U)
    {
        return false;
    }

    HR_BENCHMARK_REG32(HR_BENCHMARK_DEMCR_ADDRESS) |=
        HR_BENCHMARK_DEMCR_TRCENA;

    if ((HR_BENCHMARK_REG32(HR_BENCHMARK_DWT_CTRL_ADDRESS) &
         HR_BENCHMARK_DWT_NOCYCCNT) != 0U)
    {
        return false;
    }

    HR_BENCHMARK_REG32(HR_BENCHMARK_DWT_CYCCNT_ADDRESS) = 0U;
    HR_BENCHMARK_REG32(HR_BENCHMARK_DWT_CTRL_ADDRESS) |=
        HR_BENCHMARK_DWT_CYCCNTENA;

    __asm volatile ("dsb" ::: "memory");
    __asm volatile ("isb" ::: "memory");

    before = HR_BENCHMARK_REG32(HR_BENCHMARK_DWT_CYCCNT_ADDRESS);
    for (index = 0U; index < 32U; index++)
    {
        __asm volatile ("nop");
    }
    after = HR_BENCHMARK_REG32(HR_BENCHMARK_DWT_CYCCNT_ADDRESS);

    if (after == before)
    {
        return false;
    }

    g_benchmark_core_clock_hz = core_clock_hz;
    return true;
}

uint32_t hr_benchmark_clock_now(void)
{
    return HR_BENCHMARK_REG32(HR_BENCHMARK_DWT_CYCCNT_ADDRESS);
}

uint32_t hr_benchmark_clock_frequency_hz(void)
{
    return g_benchmark_core_clock_hz;
}
