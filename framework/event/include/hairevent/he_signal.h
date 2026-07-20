#ifndef HE_SIGNAL_H
#define HE_SIGNAL_H
#include <stdint.h>
typedef uint16_t he_signal_t;
typedef enum { HE_SIG_NONE=0, HE_SIG_ENTRY, HE_SIG_EXIT, HE_SIG_INIT, HE_SIG_TIMEOUT, HE_SIG_USER } he_reserved_signal_t;
#endif
