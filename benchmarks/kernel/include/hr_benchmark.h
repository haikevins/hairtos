#ifndef HR_BENCHMARK_H
#define HR_BENCHMARK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HR_BENCHMARK_MAX_SAMPLES 64U

typedef struct
{
    uint32_t samples[HR_BENCHMARK_MAX_SAMPLES];
    uint64_t total_cycles;
    uint32_t minimum_cycles;
    uint32_t maximum_cycles;
    size_t sample_count;
} hr_benchmark_stats_t;

void hr_benchmark_stats_reset(hr_benchmark_stats_t *stats);
bool hr_benchmark_stats_record(hr_benchmark_stats_t *stats, uint32_t cycles);
size_t hr_benchmark_stats_count(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_min(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_max(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_mean(const hr_benchmark_stats_t *stats);
uint32_t hr_benchmark_stats_percentile(const hr_benchmark_stats_t *stats,
                                       uint32_t percentile);

uint32_t hr_benchmark_elapsed_cycles(uint32_t start_cycles,
                                     uint32_t end_cycles);
uint32_t hr_benchmark_adjust_cycles(uint32_t raw_cycles,
                                    uint32_t measurement_overhead_cycles);
uint32_t hr_benchmark_cycles_to_nanoseconds(uint32_t cycles,
                                            uint32_t core_clock_hz);

bool hr_benchmark_clock_init(uint32_t core_clock_hz);
uint32_t hr_benchmark_clock_now(void);
uint32_t hr_benchmark_clock_frequency_hz(void);

void hr_benchmark_gpio_init(void);
void hr_benchmark_gpio_mark_begin(void);
void hr_benchmark_gpio_mark_end(void);

#endif /* HR_BENCHMARK_H */
