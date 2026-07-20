# Testing Guide

Host tests cover intrusive lists, ready queues, timeout ordering, queue ring
buffers, waiter ordering, event pools, and state transitions. Build with
`-Wall -Wextra -Werror`, AddressSanitizer, and UndefinedBehaviorSanitizer.

Target tests cover SVC startup, PendSV switching, preemption, round-robin,
ISR-safe wake-up, tick wrap, stack guards, and priority inheritance.

Stress tests run many task switches, queue contention sequences, event
allocation/release cycles, and timeout/object races.

A feature is complete only with API, implementation, validation, context rules,
timeout and ISR behavior, tests, example, documentation, return statuses, and
misuse assertions.

Test names follow `test_<unit>_<condition>_<expected_result>`.
