#ifndef HE_INTERNAL_H
#define HE_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "haievent/haievent.h"
#include "hairtos/hr_queue.h"
#include "hairtos/hr_timer.h"

#define HE_EVENT_FLAG_STATIC   UINT16_C(1)
#define HE_EVENT_FLAG_DYNAMIC  UINT16_C(2)

typedef struct
{
    unsigned char *storage;
    void *free_head;
    size_t block_size;
    size_t block_count;
    size_t free_count;
    uint32_t magic;
} he_event_pool_control_block_t;

typedef struct
{
    he_state_handler_t initial;
    he_state_handler_t current;
    he_state_handler_t target;
    void *context;
    uint32_t magic;
    bool started;
} he_state_machine_control_block_t;

typedef struct
{
    hr_task_t task;
    hr_queue_t queue;
    he_state_machine_t state_machine;
    const char *name;
    uint32_t magic;
} he_active_control_block_t;

typedef struct
{
    hr_timer_t timer;
    he_event_t event;
    he_active_t *target;
    const char *name;
    uint32_t dropped_count;
    uint32_t magic;
} he_time_event_control_block_t;

typedef struct
{
    he_active_t **subscribers;
    size_t signal_count;
    size_t max_subscribers;
    uint32_t magic;
} he_pubsub_control_block_t;

_Static_assert(sizeof(he_event_pool_control_block_t) <= sizeof(he_event_pool_t),
               "HE_CFG_EVENT_POOL_STORAGE_BYTES is too small");
_Static_assert(sizeof(he_state_machine_control_block_t) <= sizeof(he_state_machine_t),
               "HE_CFG_STATE_MACHINE_STORAGE_BYTES is too small");
_Static_assert(sizeof(he_active_control_block_t) <= sizeof(he_active_t),
               "HE_CFG_ACTIVE_STORAGE_BYTES is too small");
_Static_assert(sizeof(he_time_event_control_block_t) <= sizeof(he_time_event_t),
               "HE_CFG_TIME_EVENT_STORAGE_BYTES is too small");
_Static_assert(sizeof(he_pubsub_control_block_t) <= sizeof(he_pubsub_t),
               "HE_CFG_PUBSUB_STORAGE_BYTES is too small");

he_event_pool_control_block_t *he_event_pool_control_block(he_event_pool_t *pool);
const he_event_pool_control_block_t *he_event_pool_control_block_const(
    const he_event_pool_t *pool);
he_state_machine_control_block_t *he_state_machine_control_block(
    he_state_machine_t *machine);
const he_state_machine_control_block_t *he_state_machine_control_block_const(
    const he_state_machine_t *machine);
he_active_control_block_t *he_active_control_block(he_active_t *active);
const he_active_control_block_t *he_active_control_block_const(
    const he_active_t *active);
he_time_event_control_block_t *he_time_event_control_block(
    he_time_event_t *time_event);
const he_time_event_control_block_t *he_time_event_control_block_const(
    const he_time_event_t *time_event);
he_pubsub_control_block_t *he_pubsub_control_block(he_pubsub_t *pubsub);
const he_pubsub_control_block_t *he_pubsub_control_block_const(
    const he_pubsub_t *pubsub);

hr_status_t he_active_post_shared(he_active_t *active,
                                  he_event_t *event,
                                  hr_tick_t timeout);

#endif /* HE_INTERNAL_H */
