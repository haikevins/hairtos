#ifndef HE_PUBSUB_H
#define HE_PUBSUB_H

#include <stdbool.h>
#include <stddef.h>

#include "hairtos/hr_status.h"
#include "he_active.h"
#include "he_event.h"
#include "he_types.h"

hr_status_t he_pubsub_init(he_pubsub_t *pubsub,
                           he_active_t **subscriber_storage,
                           size_t signal_count,
                           size_t max_subscribers_per_signal);
bool he_pubsub_is_valid(const he_pubsub_t *pubsub);

hr_status_t he_pubsub_subscribe(he_pubsub_t *pubsub,
                                he_signal_t signal,
                                he_active_t *subscriber);
hr_status_t he_pubsub_unsubscribe(he_pubsub_t *pubsub,
                                  he_signal_t signal,
                                  he_active_t *subscriber);
size_t he_pubsub_get_subscriber_count(const he_pubsub_t *pubsub,
                                      he_signal_t signal);

/* Publishing consumes the publisher's dynamic-event reference, even when no
 * subscriber accepts the event. Static events remain owned by the caller. */
hr_status_t he_pubsub_publish(he_pubsub_t *pubsub,
                              he_event_t *event,
                              hr_tick_t timeout,
                              size_t *delivered_count);

#endif /* HE_PUBSUB_H */
