#ifndef HE_TYPES_H
#define HE_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "haievent_config.h"
#include "hairtos/hr_types.h"

typedef uint16_t he_signal_t;

typedef union he_event_pool
{
    max_align_t alignment;
    unsigned char storage[HE_CFG_EVENT_POOL_STORAGE_BYTES];
} he_event_pool_t;

typedef union he_state_machine
{
    max_align_t alignment;
    unsigned char storage[HE_CFG_STATE_MACHINE_STORAGE_BYTES];
} he_state_machine_t;

typedef union he_active
{
    max_align_t alignment;
    unsigned char storage[HE_CFG_ACTIVE_STORAGE_BYTES];
} he_active_t;

typedef union he_time_event
{
    max_align_t alignment;
    unsigned char storage[HE_CFG_TIME_EVENT_STORAGE_BYTES];
} he_time_event_t;

typedef union he_pubsub
{
    max_align_t alignment;
    unsigned char storage[HE_CFG_PUBSUB_STORAGE_BYTES];
} he_pubsub_t;

#endif /* HE_TYPES_H */
