#ifndef HR_QUEUE_H
#define HR_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#include "hr_status.h"
#include "hr_types.h"

hr_status_t hr_queue_create_static(hr_queue_t *queue,
                                   void *storage,
                                   size_t item_size,
                                   size_t capacity);

bool hr_queue_is_valid(const hr_queue_t *queue);
size_t hr_queue_get_count(const hr_queue_t *queue);
size_t hr_queue_get_capacity(const hr_queue_t *queue);
size_t hr_queue_get_waiting_senders(const hr_queue_t *queue);
size_t hr_queue_get_waiting_receivers(const hr_queue_t *queue);

hr_status_t hr_queue_send(hr_queue_t *queue,
                          const void *item,
                          hr_tick_t timeout);
hr_status_t hr_queue_receive(hr_queue_t *queue,
                             void *item,
                             hr_tick_t timeout);

hr_status_t hr_queue_send_from_isr(hr_queue_t *queue,
                                   const void *item,
                                   bool *higher_priority_task_woken);
hr_status_t hr_queue_receive_from_isr(hr_queue_t *queue,
                                      void *item,
                                      bool *higher_priority_task_woken);

#endif /* HR_QUEUE_H */
