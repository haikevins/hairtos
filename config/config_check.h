#ifndef HAIRTOS_CONFIG_CHECK_H
#define HAIRTOS_CONFIG_CHECK_H
#include "hairtos_config.h"
#include "hairevent_config.h"
#if (HR_CFG_TICK_RATE_HZ == 0U)
#error "HR_CFG_TICK_RATE_HZ must be greater than zero"
#endif
#if (HR_CFG_PRIORITY_COUNT == 0U) || (HR_CFG_PRIORITY_COUNT > 32U)
#error "HR_CFG_PRIORITY_COUNT must be in range 1..32"
#endif
#if (HR_CFG_SINGLE_CORE != 1)
#error "Phase 0 specifies a single-core kernel"
#endif
#endif
