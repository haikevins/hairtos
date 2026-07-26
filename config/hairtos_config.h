#ifndef HAIRTOS_CONFIG_H
#define HAIRTOS_CONFIG_H
#define HR_CFG_TICK_RATE_HZ              1000U
#define HR_CFG_PRIORITY_COUNT            8U
#define HR_CFG_IDLE_PRIORITY             (HR_CFG_PRIORITY_COUNT - 1U)
#define HR_CFG_MAX_TASKS                 8U
#ifndef HR_CFG_PREEMPTION
#define HR_CFG_PREEMPTION                1
#endif
#ifndef HR_CFG_TIME_SLICING
#define HR_CFG_TIME_SLICING              1
#endif
#ifndef HR_CFG_TIME_SLICE_TICKS
#define HR_CFG_TIME_SLICE_TICKS          1U
#endif
#define HR_CFG_STATIC_ALLOCATION         1
#define HR_CFG_DYNAMIC_ALLOCATION        0
#define HR_CFG_ENABLE_QUEUE              1
#define HR_CFG_ENABLE_SEMAPHORE          1
#define HR_CFG_ENABLE_MUTEX              1
#define HR_CFG_ENABLE_SOFTWARE_TIMER     1
#define HR_CFG_ENABLE_EVENT_FLAGS        0
#define HR_CFG_ENABLE_ASSERT             1
#define HR_CFG_ENABLE_STACK_CHECK        1
#define HR_CFG_ENABLE_RUNTIME_STATS      0
#define HR_CFG_STACK_FILL_BYTE           0xA5U
#define HR_CFG_STACK_GUARD_VALUE         0xDEADBEEFU
#define HR_CFG_IDLE_STACK_WORDS          128U
#define HR_CFG_TASK_STORAGE_BYTES        320U
#define HR_CFG_MIN_TASK_STACK_WORDS      32U
#define HR_CFG_TASK_MAGIC                0x48525453UL
#define HR_CFG_USE_FPU                   0
#define HR_CFG_USE_MPU                   0
#define HR_CFG_SINGLE_CORE               1
#define HR_WAIT_FOREVER                  0xFFFFFFFFUL
#define HR_NO_WAIT                       0UL
#endif
