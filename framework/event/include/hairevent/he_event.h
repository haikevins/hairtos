#ifndef HE_EVENT_H
#define HE_EVENT_H
#include <stdint.h>
#include "he_signal.h"
typedef struct { he_signal_t signal; uint16_t size; uint8_t pool_id; uint8_t reference_count; } he_event_t;
#endif
