#include <stddef.h>
#include <stdint.h>

#include "haievent/haievent.h"
#include "hairtos/hr_queue.h"
#include "hairtos/hr_task.h"
#include "he_internal.h"
#include "test_support.h"

enum
{
    TEST_SIG_GO = HE_SIG_USER,
    TEST_SIG_BACK,
    TEST_SIG_PUBLISH
};

typedef union
{
    max_align_t alignment;
    unsigned char bytes[64];
} test_event_block_t;

typedef struct
{
    he_event_t super;
    uint32_t value;
} test_value_event_t;

typedef struct
{
    uint32_t a_entry;
    uint32_t a_exit;
    uint32_t b_entry;
    uint32_t b_exit;
    uint32_t handled;
} state_context_t;

static he_state_result_t state_b(he_state_machine_t *machine,
                                 const he_event_t *event);

static he_state_result_t state_a(he_state_machine_t *machine,
                                 const he_event_t *event)
{
    state_context_t *context = (state_context_t *)he_state_machine_context(machine);

    switch (event->signal)
    {
        case HE_SIG_ENTRY:
            context->a_entry++;
            return HE_STATE_HANDLED;
        case HE_SIG_EXIT:
            context->a_exit++;
            return HE_STATE_HANDLED;
        case TEST_SIG_GO:
            return he_state_transition(machine, state_b);
        default:
            return HE_STATE_IGNORED;
    }
}

static he_state_result_t state_b(he_state_machine_t *machine,
                                 const he_event_t *event)
{
    state_context_t *context = (state_context_t *)he_state_machine_context(machine);

    switch (event->signal)
    {
        case HE_SIG_ENTRY:
            context->b_entry++;
            return HE_STATE_HANDLED;
        case HE_SIG_EXIT:
            context->b_exit++;
            return HE_STATE_HANDLED;
        case TEST_SIG_BACK:
            return he_state_transition(machine, state_a);
        case TEST_SIG_PUBLISH:
            context->handled++;
            return HE_STATE_HANDLED;
        default:
            return HE_STATE_IGNORED;
    }
}

static void dummy_task(void *argument)
{
    (void)argument;
}

static void initialize_test_active(he_active_t *active,
                                   const char *name,
                                   he_event_t **queue_storage,
                                   size_t capacity,
                                   hr_stack_t *stack)
{
    he_active_control_block_t *control_block = he_active_control_block(active);

    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_create_static(&control_block->queue,
                                               queue_storage,
                                               sizeof(queue_storage[0]),
                                               capacity));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_state_machine_init(&control_block->state_machine,
                                              state_a,
                                              NULL));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_task_create_static(&control_block->task,
                                              name,
                                              dummy_task,
                                              NULL,
                                              stack,
                                              64U,
                                              2U));
    control_block->name = name;
    control_block->magic = HE_CFG_ACTIVE_MAGIC;
    TEST_ASSERT_TRUE(he_active_is_valid(active));
}

static void test_event_pool_reference_counting(void)
{
    static he_event_pool_t pool;
    static test_event_block_t blocks[3];
    test_value_event_t *first;
    he_event_t *second;
    he_event_t static_event;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_event_pool_init(&pool,
                                           blocks,
                                           sizeof(blocks[0]),
                                           3U));
    TEST_ASSERT_EQ_UINT(3U, he_event_pool_get_free_count(&pool));

    first = (test_value_event_t *)he_event_new(&pool,
                                               TEST_SIG_GO,
                                               sizeof(*first));
    second = he_event_new(&pool, TEST_SIG_BACK, sizeof(he_event_t));
    TEST_ASSERT_TRUE(first != NULL);
    TEST_ASSERT_TRUE(second != NULL);
    first->value = 1234U;
    TEST_ASSERT_EQ_UINT(1U, he_event_get_reference_count(&first->super));
    TEST_ASSERT_EQ_UINT(1U, he_event_pool_get_free_count(&pool));

    TEST_ASSERT_EQ_UINT(HR_OK, he_event_retain(&first->super));
    TEST_ASSERT_EQ_UINT(2U, he_event_get_reference_count(&first->super));
    TEST_ASSERT_EQ_UINT(HR_OK, he_event_release(&first->super));
    TEST_ASSERT_EQ_UINT(1U, he_event_get_reference_count(&first->super));
    TEST_ASSERT_EQ_UINT(HR_OK, he_event_release(&first->super));
    TEST_ASSERT_EQ_UINT(HR_OK, he_event_release(second));
    TEST_ASSERT_EQ_UINT(3U, he_event_pool_get_free_count(&pool));

    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_event_init_static(&static_event,
                                             TEST_SIG_GO,
                                             sizeof(static_event)));
    TEST_ASSERT_TRUE(!he_event_is_dynamic(&static_event));
    TEST_ASSERT_EQ_UINT(HR_OK, he_event_retain(&static_event));
    TEST_ASSERT_EQ_UINT(HR_OK, he_event_release(&static_event));
}

static void test_flat_state_machine_entry_exit_transition(void)
{
    static he_state_machine_t machine;
    static state_context_t context;
    he_event_t go_event;
    he_event_t back_event;

    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_state_machine_init(&machine, state_a, &context));
    TEST_ASSERT_EQ_UINT(HR_OK, he_state_machine_start(&machine));
    TEST_ASSERT_EQ_UINT(1U, context.a_entry);
    TEST_ASSERT_TRUE(he_state_machine_current(&machine) == state_a);

    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_event_init_static(&go_event,
                                             TEST_SIG_GO,
                                             sizeof(go_event)));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_event_init_static(&back_event,
                                             TEST_SIG_BACK,
                                             sizeof(back_event)));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_state_machine_dispatch(&machine, &go_event));
    TEST_ASSERT_EQ_UINT(1U, context.a_exit);
    TEST_ASSERT_EQ_UINT(1U, context.b_entry);
    TEST_ASSERT_TRUE(he_state_machine_current(&machine) == state_b);

    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_state_machine_dispatch(&machine, &back_event));
    TEST_ASSERT_EQ_UINT(1U, context.b_exit);
    TEST_ASSERT_EQ_UINT(2U, context.a_entry);
    TEST_ASSERT_TRUE(he_state_machine_current(&machine) == state_a);
}

static void test_active_post_and_publish_subscribe_ownership(void)
{
    static he_active_t first_active;
    static he_active_t second_active;
    static he_event_t *first_queue[4];
    static he_event_t *second_queue[4];
    static hr_stack_t first_stack[64];
    static hr_stack_t second_stack[64];
    static he_pubsub_t bus;
    static he_active_t *subscriber_slots[HE_CFG_MAX_SIGNALS * 2U];
    static he_event_pool_t pool;
    static test_event_block_t blocks[4];
    he_event_t static_event;
    he_event_t *dynamic_event;
    he_event_t *received;
    size_t delivered = 0U;

    initialize_test_active(&first_active,
                           "first-active",
                           first_queue,
                           4U,
                           first_stack);
    initialize_test_active(&second_active,
                           "second-active",
                           second_queue,
                           4U,
                           second_stack);

    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_pubsub_init(&bus,
                                       subscriber_slots,
                                       HE_CFG_MAX_SIGNALS,
                                       2U));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_pubsub_subscribe(&bus,
                                            TEST_SIG_PUBLISH,
                                            &first_active));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_pubsub_subscribe(&bus,
                                            TEST_SIG_PUBLISH,
                                            &second_active));
    TEST_ASSERT_EQ_UINT(2U,
                        he_pubsub_get_subscriber_count(&bus,
                                                       TEST_SIG_PUBLISH));

    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_event_init_static(&static_event,
                                             TEST_SIG_PUBLISH,
                                             sizeof(static_event)));
    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_pubsub_publish(&bus,
                                          &static_event,
                                          HR_NO_WAIT,
                                          &delivered));
    TEST_ASSERT_EQ_UINT(2U, delivered);
    TEST_ASSERT_EQ_UINT(1U, he_active_get_pending_count(&first_active));
    TEST_ASSERT_EQ_UINT(1U, he_active_get_pending_count(&second_active));

    received = NULL;
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive(&he_active_control_block(&first_active)->queue,
                                         &received,
                                         HR_NO_WAIT));
    TEST_ASSERT_EQ_PTR(&static_event, received);
    received = NULL;
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive(&he_active_control_block(&second_active)->queue,
                                         &received,
                                         HR_NO_WAIT));
    TEST_ASSERT_EQ_PTR(&static_event, received);

    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_event_pool_init(&pool,
                                           blocks,
                                           sizeof(blocks[0]),
                                           4U));
    dynamic_event = he_event_new(&pool,
                                 TEST_SIG_PUBLISH,
                                 sizeof(he_event_t));
    TEST_ASSERT_TRUE(dynamic_event != NULL);
    TEST_ASSERT_EQ_UINT(HR_OK,
                        he_pubsub_publish(&bus,
                                          dynamic_event,
                                          HR_NO_WAIT,
                                          &delivered));
    TEST_ASSERT_EQ_UINT(2U, delivered);
    TEST_ASSERT_EQ_UINT(2U, he_event_get_reference_count(dynamic_event));

    received = NULL;
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive(&he_active_control_block(&first_active)->queue,
                                         &received,
                                         HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_OK, he_event_release(received));
    received = NULL;
    TEST_ASSERT_EQ_UINT(HR_OK,
                        hr_queue_receive(&he_active_control_block(&second_active)->queue,
                                         &received,
                                         HR_NO_WAIT));
    TEST_ASSERT_EQ_UINT(HR_OK, he_event_release(received));
    TEST_ASSERT_EQ_UINT(4U, he_event_pool_get_free_count(&pool));
}

void run_haievent_tests(void)
{
    RUN_TEST(test_event_pool_reference_counting);
    RUN_TEST(test_flat_state_machine_entry_exit_transition);
    RUN_TEST(test_active_post_and_publish_subscribe_ownership);
}
