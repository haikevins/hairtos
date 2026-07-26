#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hairtos_config.h"
#include "hr_scheduler_internal.h"
#include "scheduler_stress.h"

#define SCHEDULER_STRESS_NODE_COUNT 7U

typedef struct
{
    hr_ready_node_t ready;
    bool linked;
} hairtos_stress_node_t;

static uint32_t hairtos_next_random(uint32_t *state)
{
    *state = (*state * UINT32_C(1664525)) + UINT32_C(1013904223);
    return *state;
}

bool scheduler_stress_run(uint32_t iterations,
                          scheduler_stress_result_t *result)
{
    hr_scheduler_t scheduler;
    hairtos_stress_node_t nodes[SCHEDULER_STRESS_NODE_COUNT];
    uint32_t random_state = UINT32_C(0xC0FFEE11);
    uint32_t iteration;
    size_t index;

    if ((iterations == 0U) || (result == NULL))
    {
        return false;
    }

    result->iterations = iterations;
    result->insertions = 0U;
    result->removals = 0U;
    result->rotations = 0U;
    result->validations = 0U;

    hr_scheduler_init(&scheduler);
    for (index = 0U; index < SCHEDULER_STRESS_NODE_COUNT; index++)
    {
        hr_ready_node_init(&nodes[index].ready,
                           &nodes[index],
                           (hr_priority_t)(index %
                           (size_t)HR_CFG_IDLE_PRIORITY));
        nodes[index].linked = false;
    }

    for (iteration = 0U; iteration < iterations; iteration++)
    {
        const uint32_t random_value = hairtos_next_random(&random_state);
        const size_t selected =
            (size_t)(random_value % SCHEDULER_STRESS_NODE_COUNT);
        const uint32_t operation = (random_value >> 8U) % 3U;

        if (operation == 0U)
        {
            if (!nodes[selected].linked)
            {
                if (hr_scheduler_add_ready(&scheduler,
                                           &nodes[selected].ready) != HR_OK)
                {
                    return false;
                }
                nodes[selected].linked = true;
                result->insertions++;
            }
        }
        else if (operation == 1U)
        {
            if (nodes[selected].linked)
            {
                if (hr_scheduler_remove_ready(&scheduler,
                                              &nodes[selected].ready) != HR_OK)
                {
                    return false;
                }
                nodes[selected].linked = false;
                result->removals++;
            }
        }
        else
        {
            hr_ready_node_t *highest =
                hr_scheduler_select_highest(&scheduler);
            if (highest != NULL)
            {
                if (hr_scheduler_yield_current(&scheduler, highest) != HR_OK)
                {
                    return false;
                }
                result->rotations++;
            }
        }

        if (!hr_scheduler_validate(&scheduler))
        {
            return false;
        }
        result->validations++;
    }

    for (index = 0U; index < SCHEDULER_STRESS_NODE_COUNT; index++)
    {
        if (nodes[index].linked)
        {
            if (hr_scheduler_remove_ready(&scheduler,
                                          &nodes[index].ready) != HR_OK)
            {
                return false;
            }
            nodes[index].linked = false;
        }
    }

    return hr_scheduler_validate(&scheduler) &&
           (hr_scheduler_ready_count(&scheduler) == 0U);
}
