# Test matrix

> **Validation baseline:** host test suite PASS; 64 test function được compile/run trong test binary hiện tại.

[← Root README](../../README.md) · [↑ Back to section](README.md) · [← Previous](stress-testing.md) · [Next →](testing-guide.md)

## Matrix theo source file

| Test source | Số test | Trọng tâm cụ thể |
| --- | ---: | --- |
| `tests/host/test_benchmark.c` | 4 | `test_benchmark_stats_empty_state`, `test_benchmark_stats_summary_and_percentiles`, `test_benchmark_stats_capacity_is_bounded`, `test_benchmark_cycle_helpers_handle_wrap_and_conversion` |
| `tests/host/test_diagnostics.c` | 4 | `test_diagnostics_task_snapshot_and_stack_guard`, `test_diagnostics_runtime_counters_are_recorded`, `test_diagnostics_panic_record_can_be_retained_and_cleared`, `test_diagnostics_fault_record_captures_frame` |
| `tests/host/test_haievent.c` | 3 | `test_event_pool_reference_counting`, `test_flat_state_machine_entry_exit_transition`, `test_active_post_and_publish_subscribe_ownership` |
| `tests/host/test_kernel_start.c` | 1 | `test_kernel_preemption_round_robin_and_delay_race` |
| `tests/host/test_list.c` | 5 | `test_list_init_is_empty_and_valid`, `test_list_push_back_preserves_fifo`, `test_list_remove_middle_repairs_links`, `test_list_rejects_double_insert_and_double_remove`, `test_list_insert_before_places_node_correctly` |
| `tests/host/test_mutex.c` | 2 | `test_mutex_creation_and_metadata`, `test_mutex_rejects_invalid_context` |
| `tests/host/test_port_stack.c` | 3 | `test_port_stack_builds_expected_initial_frame`, `test_port_stack_aligns_top_down_to_eight_bytes`, `test_port_stack_rejects_invalid_or_small_storage` |
| `tests/host/test_queue.c` | 4 | `test_queue_fifo_wrap_full_and_empty`, `test_queue_rejects_invalid_creation_and_recreation`, `test_queue_isr_nonblocking_api`, `test_queue_blocking_requires_running_kernel` |
| `tests/host/test_ready_queue.c` | 4 | `test_ready_set_selects_smallest_priority_number`, `test_ready_set_equal_priority_is_fifo_and_rotates`, `test_ready_set_remove_clears_bitmap_bit`, `test_ready_set_rejects_invalid_priority` |
| `tests/host/test_scheduler_policy.c` | 6 | `test_scheduler_ignores_registration_order_across_priorities`, `test_scheduler_yield_rotates_only_highest_priority_fifo`, `test_scheduler_single_highest_task_yields_to_itself`, `test_scheduler_rejects_yield_from_non_selected_task`, `test_scheduler_preempts_only_for_strictly_higher_priority`, `test_scheduler_detects_equal_priority_time_slice_peer` |
| `tests/host/test_semaphore.c` | 3 | `test_counting_semaphore_nonblocking_counts`, `test_binary_semaphore_and_isr_context_rules`, `test_semaphore_rejects_invalid_creation` |
| `tests/host/test_task.c` | 5 | `test_task_create_initializes_tcb_and_stack`, `test_task_create_rejects_invalid_arguments`, `test_task_stack_guard_detects_corruption`, `test_task_high_watermark_tracks_low_stack_use`, `test_task_internal_transition_checks_expected_state` |
| `tests/host/test_timeout.c` | 4 | `test_timeout_list_expires_in_deadline_order`, `test_timeout_list_handles_tick_wrap`, `test_timeout_list_remove_cancels_timeout`, `test_timeout_list_zero_delay_expires_at_current_tick` |
| `tests/host/test_timer.c` | 2 | `test_one_shot_and_periodic_timers`, `test_timer_reset_change_period_stop_and_fifo` |
| `tests/host/test_wait_list.c` | 2 | `test_wait_list_orders_by_priority_then_fifo`, `test_wait_list_remove_unlinks_specific_waiter` |
| `labs/memory-allocator/tests/test_heap_lab.c` | 11 | `test_heap_init_alignment_and_minimum_allocation`, `test_heap_first_fit_reuses_freed_block`, `test_heap_forward_and_backward_coalescing`, `test_heap_coalesced_space_satisfies_large_allocation`, `test_heap_reports_internal_and_external_fragmentation`, `test_heap_rejects_invalid_and_double_free`, `test_heap_exhaustion_and_failed_allocation_counter`, `test_heap_randomized_allocate_free_sequences`, `test_pool_allocates_fixed_aligned_blocks_and_reuses_lifo`, `test_pool_exhaustion_stats_and_recovery`, `test_pool_rejects_invalid_and_double_free` |
| `tests/stress/test_scheduler_stress.c` | 1 | `test_scheduler_stress_preserves_invariants` |

## Những gì host tests không chứng minh

- real PendSV/SVC exception entry/return trên Cortex-M3;
- NVIC/System Handler priority interaction với application ISR;
- HSE/PLL clock startup và HSI fallback;
- UART/GPIO/hardware timer register backend trên board thật;
- DWT timing absolute dưới toolchain/hardware cụ thể;
- retained `.noinit` behavior qua power/reset thực tế.

## Evidence bổ sung

- Example 02: ready/wait policy observable on host — PASS.
- Allocator example 14 host — PASS.
- Scheduler stress example 16 host — PASS, 500.000 iteration.
- Target examples 01–16 đóng vai trò integration/learning evidence; cần Blue Pill để chạy đầy đủ.

## References

- [`testing-guide.md`](testing-guide.md)
- [`validation-baseline.md`](validation-baseline.md)
