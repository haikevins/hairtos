#include <stdio.h>
#include <stdlib.h>

#include "scheduler_stress.h"

int main(void)
{
    scheduler_stress_result_t result;

    if (!scheduler_stress_run(500000U, &result))
    {
        fprintf(stderr, "hairtos scheduler stress: FAIL\n");
        return EXIT_FAILURE;
    }

    printf("hairtos scheduler stress: PASS\n");
    printf("iterations=%u insertions=%u removals=%u rotations=%u validations=%u\n",
           result.iterations,
           result.insertions,
           result.removals,
           result.rotations,
           result.validations);

    return EXIT_SUCCESS;
}
