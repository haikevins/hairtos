
# Source ownership by module. Example selection lives in hairtos_examples.cmake.
# Hardware binding comes from cmake/targets/<target>.cmake.

set(HAIRTOS_MODULE_platform_C ${HAIRTOS_TARGET_PLATFORM_C})
set(HAIRTOS_MODULE_platform_ASM ${HAIRTOS_TARGET_PLATFORM_ASM})
set(HAIRTOS_MODULE_platform_KIND public)

set(HAIRTOS_MODULE_baremetal_tick_C ${HAIRTOS_TARGET_BAREMETAL_TICK_C})
set(HAIRTOS_MODULE_baremetal_tick_KIND public)

set(HAIRTOS_MODULE_task_kernel_C
    kernel/src/hr_list.c
    kernel/src/hr_scheduler.c
    kernel/src/hr_wait.c
    kernel/src/hr_timeout.c
    kernel/src/hr_task.c
    ${HAIRTOS_TARGET_PORT_C})
set(HAIRTOS_MODULE_task_kernel_KIND internal)

set(HAIRTOS_MODULE_kernel_runtime_C kernel/src/hr_kernel.c)
set(HAIRTOS_MODULE_kernel_runtime_ASM ${HAIRTOS_TARGET_PORT_ASM})
set(HAIRTOS_MODULE_kernel_runtime_KIND internal)

set(HAIRTOS_MODULE_kernel_time_C
    kernel/src/hr_time.c
    ${HAIRTOS_TARGET_KERNEL_TICK_C})
set(HAIRTOS_MODULE_kernel_time_KIND internal)
set(HAIRTOS_MODULE_context_C kernel/src/hr_context.c)
set(HAIRTOS_MODULE_context_KIND internal)
set(HAIRTOS_MODULE_queue_C kernel/src/hr_queue.c)
set(HAIRTOS_MODULE_queue_KIND internal)
set(HAIRTOS_MODULE_semaphore_C kernel/src/hr_semaphore.c)
set(HAIRTOS_MODULE_semaphore_KIND internal)
set(HAIRTOS_MODULE_mutex_C kernel/src/hr_mutex.c)
set(HAIRTOS_MODULE_mutex_KIND internal)
set(HAIRTOS_MODULE_timer_C kernel/src/hr_timer.c)
set(HAIRTOS_MODULE_timer_KIND internal)
set(HAIRTOS_MODULE_diagnostics_C kernel/src/hr_diagnostics.c)
set(HAIRTOS_MODULE_diagnostics_KIND internal)
set(HAIRTOS_MODULE_fault_C ${HAIRTOS_TARGET_FAULT_C})
set(HAIRTOS_MODULE_fault_ASM ${HAIRTOS_TARGET_FAULT_ASM})
set(HAIRTOS_MODULE_fault_KIND internal)

set(HAIRTOS_MODULE_haievent_C
    haievent/src/he_event.c
    haievent/src/he_state_machine.c
    haievent/src/he_active.c
    haievent/src/he_time_event.c
    haievent/src/he_pubsub.c)
set(HAIRTOS_MODULE_haievent_KIND internal)

set(HAIRTOS_MODULE_haievent_benchmark_C
    haievent/src/he_event.c
    haievent/src/he_state_machine.c
    haievent/src/he_active.c)
set(HAIRTOS_MODULE_haievent_benchmark_KIND internal)

set(HAIRTOS_MODULE_allocator_C
    labs/memory-allocator/src/hr_heap_lab.c
    labs/memory-allocator/src/hr_pool_lab.c)
set(HAIRTOS_MODULE_allocator_KIND allocator)

set(HAIRTOS_MODULE_benchmark_C
    benchmarks/kernel/src/hr_benchmark_stats.c
    ${HAIRTOS_TARGET_BENCHMARK_CLOCK_C})
set(HAIRTOS_MODULE_benchmark_KIND benchmark)

function(hairtos_collect_modules modules out_public_c out_public_asm out_internal_c out_internal_asm out_allocator_c out_benchmark_c)
    set(_public_c "")
    set(_public_asm "")
    set(_internal_c "")
    set(_internal_asm "")
    set(_allocator_c "")
    set(_benchmark_c "")

    foreach(_module IN LISTS modules)
        if(NOT DEFINED HAIRTOS_MODULE_${_module}_KIND)
            message(FATAL_ERROR "Unknown hairtos module '${_module}'")
        endif()
        set(_kind "${HAIRTOS_MODULE_${_module}_KIND}")
        if(_kind STREQUAL "public")
            list(APPEND _public_c ${HAIRTOS_MODULE_${_module}_C})
            list(APPEND _public_asm ${HAIRTOS_MODULE_${_module}_ASM})
        elseif(_kind STREQUAL "internal")
            list(APPEND _internal_c ${HAIRTOS_MODULE_${_module}_C})
            list(APPEND _internal_asm ${HAIRTOS_MODULE_${_module}_ASM})
        elseif(_kind STREQUAL "allocator")
            list(APPEND _allocator_c ${HAIRTOS_MODULE_${_module}_C})
        elseif(_kind STREQUAL "benchmark")
            list(APPEND _benchmark_c ${HAIRTOS_MODULE_${_module}_C})
        else()
            message(FATAL_ERROR "Unsupported module kind '${_kind}'")
        endif()
    endforeach()

    foreach(_list _public_c _public_asm _internal_c _internal_asm _allocator_c _benchmark_c)
        list(REMOVE_DUPLICATES ${_list})
    endforeach()

    set(${out_public_c} "${_public_c}" PARENT_SCOPE)
    set(${out_public_asm} "${_public_asm}" PARENT_SCOPE)
    set(${out_internal_c} "${_internal_c}" PARENT_SCOPE)
    set(${out_internal_asm} "${_internal_asm}" PARENT_SCOPE)
    set(${out_allocator_c} "${_allocator_c}" PARENT_SCOPE)
    set(${out_benchmark_c} "${_benchmark_c}" PARENT_SCOPE)
endfunction()
