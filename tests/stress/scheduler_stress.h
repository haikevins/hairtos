#ifndef SCHEDULER_STRESS_H
#define SCHEDULER_STRESS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t iterations;
    uint32_t insertions;
    uint32_t removals;
    uint32_t rotations;
    uint32_t validations;
} scheduler_stress_result_t;

bool scheduler_stress_run(uint32_t iterations,
                          scheduler_stress_result_t *result);

#endif /* SCHEDULER_STRESS_H */
