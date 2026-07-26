#ifndef PHASE16_STRESS_H
#define PHASE16_STRESS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t iterations;
    uint32_t insertions;
    uint32_t removals;
    uint32_t rotations;
    uint32_t validations;
} phase16_stress_result_t;

bool phase16_scheduler_stress_run(uint32_t iterations,
                                  phase16_stress_result_t *result);

#endif /* PHASE16_STRESS_H */
