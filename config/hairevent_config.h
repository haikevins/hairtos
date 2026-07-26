#ifndef HAIREVENT_CONFIG_H
#define HAIREVENT_CONFIG_H

#define HE_CFG_ENABLED                         1
#define HE_CFG_ENABLE_ACTIVE_OBJECT            1
#define HE_CFG_ENABLE_FLAT_STATE_MACHINE       1
#define HE_CFG_ENABLE_HIERARCHICAL_SM          0
#define HE_CFG_ENABLE_TIME_EVENT               1
#define HE_CFG_ENABLE_EVENT_POOL               1
#define HE_CFG_ENABLE_PUBLISH_SUBSCRIBE        1

#define HE_CFG_MAX_ACTIVE_OBJECTS              8U
#define HE_CFG_MAX_SIGNALS                     64U
#define HE_CFG_MAX_INIT_TRANSITIONS            8U

#define HE_CFG_EVENT_POOL_STORAGE_BYTES        64U
#define HE_CFG_STATE_MACHINE_STORAGE_BYTES     64U
#define HE_CFG_ACTIVE_STORAGE_BYTES            768U
#define HE_CFG_TIME_EVENT_STORAGE_BYTES        256U
#define HE_CFG_PUBSUB_STORAGE_BYTES            64U

#define HE_CFG_EVENT_MAGIC                     0x48454556UL
#define HE_CFG_EVENT_POOL_MAGIC                0x4845504CUL
#define HE_CFG_STATE_MACHINE_MAGIC             0x4845534DUL
#define HE_CFG_ACTIVE_MAGIC                    0x4845414FUL
#define HE_CFG_TIME_EVENT_MAGIC                0x48455445UL
#define HE_CFG_PUBSUB_MAGIC                    0x48455053UL

#endif /* HAIREVENT_CONFIG_H */
