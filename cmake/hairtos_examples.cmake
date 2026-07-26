# Single source of truth for example availability, modules and feature defines.

set(HAIRTOS_DEFAULT_EXAMPLE "16-diagnostics-stress-stabilization")

set(HAIRTOS_TARGET_EXAMPLES
    01-baremetal-foundation
    03-static-task-stack
    04-start-first-task
    05-cooperative-context-switch
    06-priority-scheduler
    07-task-delay-timeout
    08-preemption-round-robin
    09-queue-blocking-ipc
    10-01-semaphore-from-isr
    10-02-mutex-priority-inheritance
    11-task-suspend-resume
    12-software-timer
    13-01-event-post
    13-02-active-object
    13-03-flat-state-machine
    13-04-time-event
    13-05-publish-subscribe
    13-06-event-driven-demo
    14-memory-allocator-lab
    15-kernel-benchmark
    16-diagnostics-stress-stabilization
)

set(HAIRTOS_HOST_EXAMPLES
    02-kernel-data-structures-host
    14-memory-allocator-lab
    16-diagnostics-stress-stabilization
)

set(HAIRTOS_HOST_ONLY_EXAMPLES 02-kernel-data-structures-host)
set(HAIRTOS_DUAL_EXAMPLES
    14-memory-allocator-lab
    16-diagnostics-stress-stabilization
)

function(hairtos_validate_example environment example)
    if(environment STREQUAL "target")
        list(FIND HAIRTOS_TARGET_EXAMPLES "${example}" _index)
    elseif(environment STREQUAL "host")
        list(FIND HAIRTOS_HOST_EXAMPLES "${example}" _index)
    else()
        message(FATAL_ERROR "HAIRTOS_ENVIRONMENT must be host or target")
    endif()

    if(_index EQUAL -1)
        message(FATAL_ERROR
            "Example '${example}' is not available for environment '${environment}'. "
            "Run 'make list-examples' for the supported combinations.")
    endif()
endfunction()

function(hairtos_target_example_config example out_modules out_definitions out_main_internal)
    set(_modules platform)
    set(_definitions HR_CFG_ENABLE_SOFTWARE_TIMER=0)
    set(_main_internal FALSE)

    if(example STREQUAL "01-baremetal-foundation")
        list(APPEND _modules baremetal_tick)
    elseif(example STREQUAL "03-static-task-stack")
        list(APPEND _modules baremetal_tick task_kernel)
    elseif(example STREQUAL "04-start-first-task" OR
           example STREQUAL "05-cooperative-context-switch" OR
           example STREQUAL "06-priority-scheduler")
        list(APPEND _modules baremetal_tick task_kernel kernel_runtime)
    elseif(example STREQUAL "07-task-delay-timeout")
        list(APPEND _modules task_kernel kernel_runtime kernel_time)
        list(APPEND _definitions HR_CFG_PREEMPTION=0 HR_CFG_TIME_SLICING=0)
    elseif(example STREQUAL "08-preemption-round-robin" OR
           example STREQUAL "11-task-suspend-resume")
        list(APPEND _modules task_kernel kernel_runtime kernel_time)
    elseif(example STREQUAL "09-queue-blocking-ipc")
        list(APPEND _modules task_kernel kernel_runtime kernel_time queue)
    elseif(example STREQUAL "10-01-semaphore-from-isr")
        list(APPEND _modules task_kernel kernel_runtime kernel_time semaphore)
    elseif(example STREQUAL "10-02-mutex-priority-inheritance")
        list(APPEND _modules task_kernel kernel_runtime kernel_time mutex)
    elseif(example STREQUAL "12-software-timer")
        list(APPEND _modules task_kernel kernel_runtime kernel_time semaphore timer)
        list(REMOVE_ITEM _definitions HR_CFG_ENABLE_SOFTWARE_TIMER=0)
        list(APPEND _definitions HR_CFG_ENABLE_SOFTWARE_TIMER=1 HR_CFG_TIMER_TASK_PRIORITY=1)
    elseif(example MATCHES "^13-")
        list(APPEND _modules task_kernel kernel_runtime kernel_time context queue semaphore timer haievent)
        list(REMOVE_ITEM _definitions HR_CFG_ENABLE_SOFTWARE_TIMER=0)
        list(APPEND _definitions HR_CFG_ENABLE_SOFTWARE_TIMER=1 HR_CFG_TIMER_TASK_PRIORITY=1)
    elseif(example STREQUAL "14-memory-allocator-lab")
        list(APPEND _modules baremetal_tick allocator)
    elseif(example STREQUAL "15-kernel-benchmark")
        list(APPEND _modules task_kernel kernel_runtime kernel_time context queue semaphore mutex timer
                             haievent_benchmark benchmark)
        list(REMOVE_ITEM _definitions HR_CFG_ENABLE_SOFTWARE_TIMER=0)
        list(APPEND _definitions
            HR_CFG_PREEMPTION=1
            HR_CFG_TIME_SLICING=0
            HR_CFG_ENABLE_SOFTWARE_TIMER=1
            HR_CFG_TIMER_TASK_PRIORITY=1)
        set(_main_internal TRUE)
    elseif(example STREQUAL "16-diagnostics-stress-stabilization")
        list(APPEND _modules task_kernel kernel_runtime kernel_time context queue semaphore mutex timer
                             diagnostics fault)
        list(REMOVE_ITEM _definitions HR_CFG_ENABLE_SOFTWARE_TIMER=0)
        list(APPEND _definitions
            HR_CFG_PREEMPTION=1
            HR_CFG_TIME_SLICING=1
            HR_CFG_ENABLE_SOFTWARE_TIMER=1
            HR_CFG_TIMER_TASK_PRIORITY=1
            HR_CFG_ENABLE_DIAGNOSTICS=1
            HR_CFG_ENABLE_RUNTIME_STATS=1)
    else()
        message(FATAL_ERROR "No target module configuration for '${example}'")
    endif()

    list(REMOVE_DUPLICATES _modules)
    list(REMOVE_DUPLICATES _definitions)
    set(${out_modules} "${_modules}" PARENT_SCOPE)
    set(${out_definitions} "${_definitions}" PARENT_SCOPE)
    set(${out_main_internal} "${_main_internal}" PARENT_SCOPE)
endfunction()

function(hairtos_host_example_config example out_sources out_internal out_extra_includes)
    set(_sources "")
    set(_internal FALSE)
    set(_extra_includes "")

    if(example STREQUAL "02-kernel-data-structures-host")
        set(_sources
            kernel/src/hr_list.c
            kernel/src/hr_scheduler.c
            kernel/src/hr_wait.c
            examples/02-kernel-data-structures-host/main.c)
        set(_internal TRUE)
    elseif(example STREQUAL "14-memory-allocator-lab")
        set(_sources
            labs/memory-allocator/src/hr_heap_lab.c
            labs/memory-allocator/src/hr_pool_lab.c
            labs/memory-allocator/demo.c)
        list(APPEND _extra_includes labs/memory-allocator/include)
    elseif(example STREQUAL "16-diagnostics-stress-stabilization")
        set(_sources
            kernel/src/hr_list.c
            kernel/src/hr_scheduler.c
            tests/stress/scheduler_stress_core.c
            tests/stress/scheduler_stress_main.c)
        set(_internal TRUE)
        list(APPEND _extra_includes tests/stress)
    else()
        message(FATAL_ERROR "No host source configuration for '${example}'")
    endif()

    set(${out_sources} "${_sources}" PARENT_SCOPE)
    set(${out_internal} "${_internal}" PARENT_SCOPE)
    set(${out_extra_includes} "${_extra_includes}" PARENT_SCOPE)
endfunction()
