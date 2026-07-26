#include <stdio.h>
#include <stdlib.h>

#include "phase16_stress.h"

int main(void)
{
    phase16_stress_result_t result;

    if (!phase16_scheduler_stress_run(500000U, &result))
    {
        fprintf(stderr, "HairRTOS Phase 16 scheduler stress: FAIL\n");
        return EXIT_FAILURE;
    }

    printf("HairRTOS Phase 16 scheduler stress: PASS\n");
    printf("iterations=%u insertions=%u removals=%u rotations=%u validations=%u\n",
           result.iterations,
           result.insertions,
           result.removals,
           result.rotations,
           result.validations);
    return EXIT_SUCCESS;
}
