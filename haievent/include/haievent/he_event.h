#ifndef HE_EVENT_H
#define HE_EVENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hairtos/hr_status.h"
#include "he_types.h"

enum
{
    HE_SIG_NONE = 0,
    HE_SIG_ENTRY = 1,
    HE_SIG_EXIT = 2,
    HE_SIG_INIT = 3,
    HE_SIG_TIMEOUT = 4,
    HE_SIG_USER = 32
};

typedef struct he_event
{
    void *pool;
    uint32_t magic;
    uint16_t size;
    uint16_t reference_count;
    he_signal_t signal;
    uint16_t flags;
} he_event_t;

hr_status_t he_event_pool_init(he_event_pool_t *pool,
                               void *storage,
                               size_t block_size,
                               size_t block_count);
bool he_event_pool_is_valid(const he_event_pool_t *pool);
size_t he_event_pool_get_free_count(const he_event_pool_t *pool);
size_t he_event_pool_get_block_count(const he_event_pool_t *pool);

he_event_t *he_event_new(he_event_pool_t *pool,
                         he_signal_t signal,
                         size_t event_size);
hr_status_t he_event_init_static(he_event_t *event,
                                 he_signal_t signal,
                                 size_t event_size);
hr_status_t he_event_retain(he_event_t *event);
hr_status_t he_event_release(he_event_t *event);

bool he_event_is_valid(const he_event_t *event);
bool he_event_is_dynamic(const he_event_t *event);
uint16_t he_event_get_reference_count(const he_event_t *event);

#endif /* HE_EVENT_H */
