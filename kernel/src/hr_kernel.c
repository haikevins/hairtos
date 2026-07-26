#include <stddef.h>

#include "hairtos_config.h"
#include "hairtos/hr_kernel.h"
#include "hairtos/hr_task.h"
#include "hr_kernel_internal.h"
#include "hr_list_internal.h"
#include "hr_port.h"
#include "hr_scheduler_internal.h"
#include "hr_task_internal.h"

static hr_kernel_state_t g_kernel_state = HR_KERNEL_STATE_RESET;
static hr_ready_set_t g_ready_set;
static hr_list_t g_all_tasks;
static hr_task_t *g_current_task;
static size_t g_task_count;
static hr_tick_t g_kernel_tick;

static hr_task_t g_idle_task;
static hr_stack_t g_idle_stack[HR_CFG_IDLE_STACK_WORDS];

hr_task_control_block_t *g_hr_current_task_control_block;

static void hr_idle_task(void *argument)
{
    (void)argument;

    for (;;)
    {
        hr_port_wait_for_interrupt();
    }
}

hr_status_t hr_kernel_init(void)
{
    hr_status_t status;

    if (g_kernel_state != HR_KERNEL_STATE_RESET)
    {
        return HR_ERROR_INVALID_STATE;
    }

    hr_ready_set_init(&g_ready_set);
    hr_list_init(&g_all_tasks);
    g_current_task = NULL;
    g_hr_current_task_control_block = NULL;
    g_task_count = 0U;
    g_kernel_tick = 0U;
    g_kernel_state = HR_KERNEL_STATE_INITIALIZED;

    status = hr_task_create_static(&g_idle_task,
                                   "idle",
                                   hr_idle_task,
                                   NULL,
                                   g_idle_stack,
                                   HR_CFG_IDLE_STACK_WORDS,
                                   (hr_priority_t)(HR_CFG_PRIORITY_COUNT - 1U));
    if (status != HR_OK)
    {
        g_kernel_state = HR_KERNEL_STATE_PANIC;
        return status;
    }

    status = hr_task_start(&g_idle_task);
    if (status != HR_OK)
    {
        g_kernel_state = HR_KERNEL_STATE_PANIC;
        return status;
    }

    return HR_OK;
}

hr_status_t hr_kernel_register_task(hr_task_t *task)
{
    hr_task_control_block_t *control_block;
    hr_status_t status;

    if ((g_kernel_state != HR_KERNEL_STATE_INITIALIZED) || !hr_task_is_valid(task))
    {
        return HR_ERROR_INVALID_STATE;
    }

    if (g_task_count >= (size_t)HR_CFG_MAX_TASKS)
    {
        return HR_ERROR_NO_MEMORY;
    }

    control_block = hr_task_control_block(task);
    if ((control_block->state != HR_TASK_STATE_CREATED) ||
        hr_list_node_is_linked(&control_block->ready_node.node) ||
        hr_list_node_is_linked(&control_block->all_task_node))
    {
        return HR_ERROR_INVALID_STATE;
    }

    status = hr_ready_set_insert(&g_ready_set, &control_block->ready_node);
    if (status != HR_OK)
    {
        return status;
    }

    status = hr_list_push_back(&g_all_tasks, &control_block->all_task_node);
    if (status != HR_OK)
    {
        (void)hr_ready_set_remove(&g_ready_set, &control_block->ready_node);
        return status;
    }

    control_block->state = HR_TASK_STATE_READY;
    g_task_count++;
    return HR_OK;
}

hr_status_t hr_kernel_prepare_start(void)
{
    hr_ready_node_t *ready_node;
    hr_task_t *selected_task;
    hr_task_control_block_t *control_block;

    if (g_kernel_state != HR_KERNEL_STATE_INITIALIZED)
    {
        return HR_ERROR_INVALID_STATE;
    }

    ready_node = hr_ready_set_peek_highest(&g_ready_set);
    if (ready_node == NULL)
    {
        return HR_ERROR_INTERNAL;
    }

    selected_task = (hr_task_t *)hr_list_node_owner(&ready_node->node);
    if (!hr_task_is_valid(selected_task))
    {
        return HR_ERROR_INTERNAL;
    }

    control_block = hr_task_control_block(selected_task);
    if (control_block->state != HR_TASK_STATE_READY)
    {
        return HR_ERROR_INVALID_STATE;
    }

    control_block->state = HR_TASK_STATE_RUNNING;
    g_current_task = selected_task;
    g_hr_current_task_control_block = control_block;
    g_kernel_state = HR_KERNEL_STATE_RUNNING;
    return HR_OK;
}


void hr_kernel_select_next_from_pendsv(void)
{
    hr_ready_node_t *current_ready_node;
    hr_ready_node_t *next_ready_node;
    hr_task_t *next_task;
    hr_task_control_block_t *current_control_block;
    hr_task_control_block_t *next_control_block;
    hr_status_t status;

    if ((g_kernel_state != HR_KERNEL_STATE_RUNNING) ||
        (g_current_task == NULL) ||
        (g_hr_current_task_control_block == NULL))
    {
        g_kernel_state = HR_KERNEL_STATE_PANIC;
        return;
    }

    current_ready_node = hr_ready_set_peek_highest(&g_ready_set);
    if ((current_ready_node == NULL) ||
        (hr_list_node_owner(&current_ready_node->node) != g_current_task))
    {
        g_kernel_state = HR_KERNEL_STATE_PANIC;
        return;
    }

    status = hr_ready_set_rotate_highest(&g_ready_set);
    if (status != HR_OK)
    {
        g_kernel_state = HR_KERNEL_STATE_PANIC;
        return;
    }

    next_ready_node = hr_ready_set_peek_highest(&g_ready_set);
    if (next_ready_node == NULL)
    {
        g_kernel_state = HR_KERNEL_STATE_PANIC;
        return;
    }

    next_task = (hr_task_t *)hr_list_node_owner(&next_ready_node->node);
    if (!hr_task_is_valid(next_task))
    {
        g_kernel_state = HR_KERNEL_STATE_PANIC;
        return;
    }

    current_control_block = hr_task_control_block(g_current_task);
    next_control_block = hr_task_control_block(next_task);

    if (next_task != g_current_task)
    {
        if ((current_control_block->state != HR_TASK_STATE_RUNNING) ||
            (next_control_block->state != HR_TASK_STATE_READY))
        {
            g_kernel_state = HR_KERNEL_STATE_PANIC;
            return;
        }

        current_control_block->state = HR_TASK_STATE_READY;
        next_control_block->state = HR_TASK_STATE_RUNNING;
    }

    g_current_task = next_task;
    g_hr_current_task_control_block = next_control_block;
}

hr_status_t hr_kernel_start(void)
{
    hr_status_t status;

    hr_port_configure_kernel_exceptions();

    status = hr_kernel_prepare_start();
    if (status != HR_OK)
    {
        return status;
    }

    hr_port_start_first_task();

    /* A successful SVC startup never returns here. */
    g_kernel_state = HR_KERNEL_STATE_PANIC;
    return HR_ERROR_INTERNAL;
}

bool hr_kernel_is_running(void)
{
    return g_kernel_state == HR_KERNEL_STATE_RUNNING;
}

hr_kernel_state_t hr_kernel_get_state(void)
{
    return g_kernel_state;
}

hr_tick_t hr_kernel_get_tick(void)
{
    return g_kernel_tick;
}

size_t hr_kernel_get_task_count(void)
{
    return g_task_count;
}

hr_task_t *hr_kernel_current_task_internal(void)
{
    return g_current_task;
}
