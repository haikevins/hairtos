#ifndef HAIRTOS_CONFIG_CHECK_H
#define HAIRTOS_CONFIG_CHECK_H
#include "hairtos_config.h"
#include "haievent_config.h"
#if (HR_CFG_TICK_RATE_HZ == 0U)
#error "HR_CFG_TICK_RATE_HZ must be greater than zero"
#endif
#if (HR_CFG_PRIORITY_COUNT == 0U) || (HR_CFG_PRIORITY_COUNT > 32U)
#error "HR_CFG_PRIORITY_COUNT must be in range 1..32"
#endif
#if (HR_CFG_IDLE_PRIORITY >= HR_CFG_PRIORITY_COUNT)
#error "HR_CFG_IDLE_PRIORITY must be inside the configured priority range"
#endif
#if (HR_CFG_TASK_STORAGE_BYTES < 128U)
#error "HR_CFG_TASK_STORAGE_BYTES is too small"
#endif

#if (HR_CFG_QUEUE_STORAGE_BYTES < 128U)
#error "HR_CFG_QUEUE_STORAGE_BYTES is too small"
#endif

#if (HR_CFG_SEMAPHORE_STORAGE_BYTES < 64U)
#error "HR_CFG_SEMAPHORE_STORAGE_BYTES is too small"
#endif

#if (HR_CFG_MUTEX_STORAGE_BYTES < 96U)
#error "HR_CFG_MUTEX_STORAGE_BYTES is too small"
#endif

#if (HR_CFG_MIN_TASK_STACK_WORDS < 18U)
#error "HR_CFG_MIN_TASK_STACK_WORDS is too small for Cortex-M3"
#endif

#if (HR_CFG_PREEMPTION != 0) && (HR_CFG_PREEMPTION != 1)
#error "HR_CFG_PREEMPTION must be 0 or 1"
#endif
#if (HR_CFG_TIME_SLICING != 0) && (HR_CFG_TIME_SLICING != 1)
#error "HR_CFG_TIME_SLICING must be 0 or 1"
#endif
#if (HR_CFG_TIME_SLICING == 1) && (HR_CFG_TIME_SLICE_TICKS == 0U)
#error "HR_CFG_TIME_SLICE_TICKS must be greater than zero when time slicing is enabled"
#endif

#if (HE_CFG_ENABLED != 0) && (HE_CFG_ENABLED != 1)
#error "HE_CFG_ENABLED must be 0 or 1"
#endif
#if (HE_CFG_MAX_ACTIVE_OBJECTS == 0U)
#error "HE_CFG_MAX_ACTIVE_OBJECTS must be greater than zero"
#endif
#if (HE_CFG_MAX_SIGNALS <= 32U)
/* Reserved signals occupy values below the first application signal. */
#error "HE_CFG_MAX_SIGNALS must be greater than 32"
#endif
#if (HE_CFG_ACTIVE_STORAGE_BYTES < 512U)
#error "HE_CFG_ACTIVE_STORAGE_BYTES is too small"
#endif

#if (HR_CFG_SINGLE_CORE != 1)
#error "hairtos supports a single-core kernel"
#endif
#endif
