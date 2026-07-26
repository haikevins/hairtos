#include "hr_benchmark.h"

#include <limits.h>

static uint64_t hr_benchmark_divide_u64_by_u32(uint64_t numerator,
                                                uint32_t denominator)
{
    uint64_t quotient = 0U;
    uint64_t remainder = 0U;
    uint32_t bit;

    if (denominator == 0U)
    {
        return 0U;
    }

    for (bit = 0U; bit < 64U; bit++)
    {
        remainder = (remainder << 1U) | (numerator >> 63U);
        numerator <<= 1U;
        quotient <<= 1U;
        if (remainder >= (uint64_t)denominator)
        {
            remainder -= (uint64_t)denominator;
            quotient |= UINT64_C(1);
        }
    }

    return quotient;
}

void hr_benchmark_stats_reset(hr_benchmark_stats_t *stats)
{
    size_t index;

    if (stats == NULL)
    {
        return;
    }

    for (index = 0U; index < (size_t)HR_BENCHMARK_MAX_SAMPLES; index++)
    {
        stats->samples[index] = 0U;
    }

    stats->total_cycles = 0U;
    stats->minimum_cycles = UINT32_MAX;
    stats->maximum_cycles = 0U;
    stats->sample_count = 0U;
}

bool hr_benchmark_stats_record(hr_benchmark_stats_t *stats, uint32_t cycles)
{
    if ((stats == NULL) ||
        (stats->sample_count >= (size_t)HR_BENCHMARK_MAX_SAMPLES))
    {
        return false;
    }

    stats->samples[stats->sample_count] = cycles;
    stats->sample_count++;
    stats->total_cycles += (uint64_t)cycles;

    if (cycles < stats->minimum_cycles)
    {
        stats->minimum_cycles = cycles;
    }
    if (cycles > stats->maximum_cycles)
    {
        stats->maximum_cycles = cycles;
    }

    return true;
}

size_t hr_benchmark_stats_count(const hr_benchmark_stats_t *stats)
{
    return (stats == NULL) ? 0U : stats->sample_count;
}

uint32_t hr_benchmark_stats_min(const hr_benchmark_stats_t *stats)
{
    return ((stats == NULL) || (stats->sample_count == 0U)) ?
           0U : stats->minimum_cycles;
}

uint32_t hr_benchmark_stats_max(const hr_benchmark_stats_t *stats)
{
    return ((stats == NULL) || (stats->sample_count == 0U)) ?
           0U : stats->maximum_cycles;
}

uint32_t hr_benchmark_stats_mean(const hr_benchmark_stats_t *stats)
{
    if ((stats == NULL) || (stats->sample_count == 0U))
    {
        return 0U;
    }

    return (uint32_t)hr_benchmark_divide_u64_by_u32(
        stats->total_cycles,
        (uint32_t)stats->sample_count);
}

static void hr_benchmark_sort_samples(uint32_t *samples, size_t count)
{
    size_t index;

    for (index = 1U; index < count; index++)
    {
        const uint32_t value = samples[index];
        size_t position = index;

        while ((position > 0U) && (samples[position - 1U] > value))
        {
            samples[position] = samples[position - 1U];
            position--;
        }
        samples[position] = value;
    }
}

uint32_t hr_benchmark_stats_percentile(const hr_benchmark_stats_t *stats,
                                       uint32_t percentile)
{
    uint32_t sorted[HR_BENCHMARK_MAX_SAMPLES];
    size_t index;
    size_t rank;

    if ((stats == NULL) || (stats->sample_count == 0U))
    {
        return 0U;
    }

    if (percentile > 100U)
    {
        percentile = 100U;
    }

    for (index = 0U; index < stats->sample_count; index++)
    {
        sorted[index] = stats->samples[index];
    }
    hr_benchmark_sort_samples(sorted, stats->sample_count);

    if (percentile == 0U)
    {
        return sorted[0];
    }

    rank = (((size_t)percentile * stats->sample_count) + 99U) / 100U;
    if (rank == 0U)
    {
        rank = 1U;
    }
    if (rank > stats->sample_count)
    {
        rank = stats->sample_count;
    }

    return sorted[rank - 1U];
}

uint32_t hr_benchmark_elapsed_cycles(uint32_t start_cycles,
                                     uint32_t end_cycles)
{
    return end_cycles - start_cycles;
}

uint32_t hr_benchmark_adjust_cycles(uint32_t raw_cycles,
                                    uint32_t measurement_overhead_cycles)
{
    return (raw_cycles > measurement_overhead_cycles) ?
           (raw_cycles - measurement_overhead_cycles) : 0U;
}

uint32_t hr_benchmark_cycles_to_nanoseconds(uint32_t cycles,
                                            uint32_t core_clock_hz)
{
    uint64_t nanoseconds;

    if (core_clock_hz == 0U)
    {
        return 0U;
    }

    nanoseconds = hr_benchmark_divide_u64_by_u32(
        (uint64_t)cycles * UINT64_C(1000000000),
        core_clock_hz);
    if (nanoseconds > (uint64_t)UINT32_MAX)
    {
        return UINT32_MAX;
    }

    return (uint32_t)nanoseconds;
}
