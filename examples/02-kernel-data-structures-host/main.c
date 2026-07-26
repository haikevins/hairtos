#include <stdio.h>
#include <stdlib.h>

#include "hr_scheduler_internal.h"
#include "hr_wait_internal.h"

typedef struct
{
    const char *name;
    hr_ready_node_t ready;
    hr_wait_node_t wait;
} demo_task_t;

static void demo_task_init(demo_task_t *task,
                           const char *name,
                           hr_priority_t priority)
{
    task->name = name;
    hr_ready_node_init(&task->ready, task, priority);
    hr_wait_node_init(&task->wait, task, priority);
}

static demo_task_t *owner_from_ready(hr_ready_node_t *node)
{
    return (demo_task_t *)hr_list_node_owner(&node->node);
}

static demo_task_t *owner_from_wait(hr_wait_node_t *node)
{
    return (demo_task_t *)hr_list_node_owner(&node->node);
}

int main(void)
{
    hr_ready_set_t ready_set;
    hr_wait_list_t wait_list;
    demo_task_t sensor_a;
    demo_task_t sensor_b;
    demo_task_t communication;

    hr_ready_set_init(&ready_set);
    hr_wait_list_init(&wait_list);

    demo_task_init(&sensor_a, "sensor-a", 3U);
    demo_task_init(&sensor_b, "sensor-b", 3U);
    demo_task_init(&communication, "communication", 1U);

    if ((hr_ready_set_insert(&ready_set, &sensor_a.ready) != HR_OK) ||
        (hr_ready_set_insert(&ready_set, &sensor_b.ready) != HR_OK) ||
        (hr_ready_set_insert(&ready_set, &communication.ready) != HR_OK))
    {
        return EXIT_FAILURE;
    }

    printf("highest-ready: %s\n",
           owner_from_ready(hr_ready_set_peek_highest(&ready_set))->name);

    if (hr_ready_set_remove(&ready_set, &communication.ready) != HR_OK)
    {
        return EXIT_FAILURE;
    }

    printf("equal-priority-before-rotate: %s\n",
           owner_from_ready(hr_ready_set_peek_highest(&ready_set))->name);
    if (hr_ready_set_rotate_highest(&ready_set) != HR_OK)
    {
        return EXIT_FAILURE;
    }
    printf("equal-priority-after-rotate: %s\n",
           owner_from_ready(hr_ready_set_peek_highest(&ready_set))->name);

    if ((hr_wait_list_insert(&wait_list, &sensor_a.wait) != HR_OK) ||
        (hr_wait_list_insert(&wait_list, &communication.wait) != HR_OK) ||
        (hr_wait_list_insert(&wait_list, &sensor_b.wait) != HR_OK))
    {
        return EXIT_FAILURE;
    }

    printf("first-waiter: %s\n",
           owner_from_wait(hr_wait_list_peek(&wait_list))->name);

    return (hr_ready_set_validate(&ready_set) &&
            hr_wait_list_validate(&wait_list)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
