# haievent API

## Event pool / event

```c
he_event_pool_init()
he_event_pool_get_free_count()
he_event_new()
he_event_init_static()
he_event_retain()
he_event_release()
```

Dynamic event reference count quản lý shared ownership.

## State machine

```c
he_state_machine_init()
he_state_machine_start()
he_state_machine_dispatch()
he_state_transition()
he_state_machine_current()
he_state_machine_context()
```

v1 = flat FSM.

## Active Object

```c
he_active_create_static()
he_active_post()
he_active_post_from_isr()
he_active_get_pending_count()
he_active_get_task()
he_active_get_state_machine()
```

AO create đồng thời tạo task/queue/FSM.

## Time Event

```c
he_time_event_create_static()
he_time_event_arm()
he_time_event_disarm()
he_time_event_rearm()
he_time_event_change_period()
he_time_event_get_dropped_count()
```

## Publish/Subscribe

```c
he_pubsub_init()
he_pubsub_subscribe()
he_pubsub_unsubscribe()
he_pubsub_get_subscriber_count()
he_pubsub_publish()
```

## Signal

Application signal bắt đầu từ `HE_SIG_USER`.

## Ownership rule

Dynamic event không được release tùy tiện sau successful ownership transfer. Xem [`../03-haievent/ownership-and-rtc.md`](../03-haievent/ownership-and-rtc.md).
