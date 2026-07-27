
cmake_minimum_required(VERSION 3.20)
project(hairtos LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/hairtos_targets.cmake)
include(cmake/hairtos_examples.cmake)

set(HAIRTOS_ENVIRONMENT "target" CACHE STRING "Build environment: auto, host or target")
set_property(CACHE HAIRTOS_ENVIRONMENT PROPERTY STRINGS auto host target)
set(HAIRTOS_TARGET "${HAIRTOS_DEFAULT_TARGET}" CACHE STRING "Hardware target")
set_property(CACHE HAIRTOS_TARGET PROPERTY STRINGS ${HAIRTOS_SUPPORTED_TARGETS})
set(HAIRTOS_EXAMPLE "${HAIRTOS_DEFAULT_EXAMPLE}" CACHE STRING "Example name")
set(HAIRTOS_BUILD_TESTS OFF CACHE BOOL "Build native host tests")
set(HAIRTOS_EXTRA_C_FLAGS "" CACHE STRING "Additional compiler flags")

hairtos_validate_target("${HAIRTOS_TARGET}")
include(cmake/targets/${HAIRTOS_TARGET}.cmake)
hairtos_validate_target_manifest()
include(cmake/hairtos_modules.cmake)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" HAIRTOS_VERSION_RAW)
string(STRIP "${HAIRTOS_VERSION_RAW}" HAIRTOS_VERSION)
message(STATUS "hairtos version: ${HAIRTOS_VERSION}")
message(STATUS "hairtos target: ${HAIRTOS_TARGET} (${HAIRTOS_TARGET_DESCRIPTION})")

if(HAIRTOS_ENVIRONMENT STREQUAL "auto")
    list(FIND HAIRTOS_HOST_ONLY_EXAMPLES "${HAIRTOS_EXAMPLE}" _host_only_index)
    if(_host_only_index EQUAL -1)
        set(HAIRTOS_ENVIRONMENT target)
    else()
        set(HAIRTOS_ENVIRONMENT host)
    endif()
endif()

hairtos_validate_example("${HAIRTOS_ENVIRONMENT}" "${HAIRTOS_EXAMPLE}")

set(HAIRTOS_PUBLIC_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/haievent/include)
foreach(_include IN LISTS HAIRTOS_TARGET_PUBLIC_INCLUDES)
    list(APPEND HAIRTOS_PUBLIC_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/${_include})
endforeach()

set(HAIRTOS_INTERNAL_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/haievent/internal)

if(HAIRTOS_EXTRA_C_FLAGS)
    separate_arguments(HAIRTOS_EXTRA_C_FLAGS_LIST NATIVE_COMMAND "${HAIRTOS_EXTRA_C_FLAGS}")
else()
    set(HAIRTOS_EXTRA_C_FLAGS_LIST "")
endif()

function(hairtos_apply_target_warnings target)
    target_compile_options(${target} PRIVATE
        ${HAIRTOS_TARGET_CPU_FLAGS}
        -Og -g3
        -ffreestanding -fno-common -fno-builtin
        -fno-unwind-tables -fno-asynchronous-unwind-tables
        -ffunction-sections -fdata-sections
        -Wall -Wextra -Werror -Wshadow -Wundef
        -Wconversion -Wsign-conversion
        ${HAIRTOS_EXTRA_C_FLAGS_LIST})
endfunction()

function(hairtos_apply_host_warnings target)
    target_compile_options(${target} PRIVATE
        -O0 -g3 -Wall -Wextra -Werror -Wshadow -Wundef
        -Wconversion -Wsign-conversion -pedantic
        -fsanitize=address,undefined -fno-omit-frame-pointer
        ${HAIRTOS_EXTRA_C_FLAGS_LIST})
    target_link_options(${target} PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer)
endfunction()

function(hairtos_apply_target_definitions target)
    target_compile_definitions(${target} PRIVATE
        ${HAIRTOS_TARGET_COMPILE_DEFINITIONS}
        ${HAIRTOS_EXAMPLE_DEFINITIONS})
endfunction()

if(HAIRTOS_ENVIRONMENT STREQUAL "target")
    hairtos_target_example_config(
        "${HAIRTOS_EXAMPLE}"
        HAIRTOS_SELECTED_MODULES
        HAIRTOS_EXAMPLE_DEFINITIONS
        HAIRTOS_MAIN_NEEDS_INTERNAL)

    hairtos_collect_modules(
        "${HAIRTOS_SELECTED_MODULES}"
        HAIRTOS_PUBLIC_C
        HAIRTOS_PUBLIC_ASM
        HAIRTOS_INTERNAL_C
        HAIRTOS_INTERNAL_ASM
        HAIRTOS_ALLOCATOR_C
        HAIRTOS_BENCHMARK_C)

    if(HAIRTOS_PUBLIC_C OR HAIRTOS_PUBLIC_ASM)
        add_library(hairtos_platform_objects OBJECT ${HAIRTOS_PUBLIC_C} ${HAIRTOS_PUBLIC_ASM})
        target_include_directories(hairtos_platform_objects PRIVATE ${HAIRTOS_PUBLIC_INCLUDES})
        hairtos_apply_target_definitions(hairtos_platform_objects)
        hairtos_apply_target_warnings(hairtos_platform_objects)
    endif()

    if(HAIRTOS_INTERNAL_C OR HAIRTOS_INTERNAL_ASM)
        add_library(hairtos_internal_objects OBJECT ${HAIRTOS_INTERNAL_C} ${HAIRTOS_INTERNAL_ASM})
        target_include_directories(hairtos_internal_objects PRIVATE
            ${HAIRTOS_PUBLIC_INCLUDES}
            ${HAIRTOS_INTERNAL_INCLUDES})
        hairtos_apply_target_definitions(hairtos_internal_objects)
        hairtos_apply_target_warnings(hairtos_internal_objects)
    endif()

    if(HAIRTOS_ALLOCATOR_C)
        add_library(hairtos_allocator_objects OBJECT ${HAIRTOS_ALLOCATOR_C})
        target_include_directories(hairtos_allocator_objects PRIVATE
            ${HAIRTOS_PUBLIC_INCLUDES}
            ${CMAKE_CURRENT_SOURCE_DIR}/labs/memory-allocator/include)
        hairtos_apply_target_definitions(hairtos_allocator_objects)
        hairtos_apply_target_warnings(hairtos_allocator_objects)
    endif()

    if(HAIRTOS_BENCHMARK_C)
        add_library(hairtos_benchmark_objects OBJECT ${HAIRTOS_BENCHMARK_C})
        target_include_directories(hairtos_benchmark_objects PRIVATE
            ${HAIRTOS_PUBLIC_INCLUDES}
            ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/kernel/include)
        hairtos_apply_target_definitions(hairtos_benchmark_objects)
        hairtos_apply_target_warnings(hairtos_benchmark_objects)
    endif()

    add_executable(hairtos examples/${HAIRTOS_EXAMPLE}/main.c)
    target_include_directories(hairtos PRIVATE ${HAIRTOS_PUBLIC_INCLUDES})

    if(HAIRTOS_EXAMPLE STREQUAL "14-memory-allocator-lab")
        target_include_directories(hairtos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/labs/memory-allocator/include)
    endif()
    if(HAIRTOS_EXAMPLE STREQUAL "15-kernel-benchmark")
        target_include_directories(hairtos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/kernel/include)
    endif()
    if(HAIRTOS_MAIN_NEEDS_INTERNAL)
        target_include_directories(hairtos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/kernel/internal)
    endif()

    hairtos_apply_target_definitions(hairtos)
    hairtos_apply_target_warnings(hairtos)

    foreach(object_target
            hairtos_platform_objects
            hairtos_internal_objects
            hairtos_allocator_objects
            hairtos_benchmark_objects)
        if(TARGET ${object_target})
            target_sources(hairtos PRIVATE $<TARGET_OBJECTS:${object_target}>)
        endif()
    endforeach()

    target_link_options(hairtos PRIVATE
        ${HAIRTOS_TARGET_CPU_FLAGS}
        -nostdlib
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/hairtos.map
        -Wl,--cref
        -T${CMAKE_CURRENT_SOURCE_DIR}/${HAIRTOS_TARGET_LINKER_SCRIPT})

    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_link_libraries(hairtos PRIVATE gcc)
    endif()

    set_target_properties(hairtos PROPERTIES SUFFIX ".elf")

    if(NOT CMAKE_OBJCOPY)
        find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy llvm-objcopy REQUIRED)
    endif()
    find_program(HAIRTOS_SIZE_TOOL NAMES arm-none-eabi-size llvm-size size REQUIRED)
    find_program(HAIRTOS_OBJDUMP_TOOL NAMES arm-none-eabi-objdump llvm-objdump REQUIRED)

    add_custom_command(TARGET hairtos POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:hairtos> ${CMAKE_CURRENT_BINARY_DIR}/hairtos.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:hairtos> ${CMAKE_CURRENT_BINARY_DIR}/hairtos.hex
        VERBATIM)

    add_custom_target(size
        COMMAND ${HAIRTOS_SIZE_TOOL} $<TARGET_FILE:hairtos>
        DEPENDS hairtos
        VERBATIM)

    add_custom_target(disasm
        COMMAND ${HAIRTOS_OBJDUMP_TOOL} -d -S $<TARGET_FILE:hairtos> > ${CMAKE_CURRENT_BINARY_DIR}/hairtos.lst
        DEPENDS hairtos
        VERBATIM)

elseif(HAIRTOS_ENVIRONMENT STREQUAL "host")
    if(HAIRTOS_BUILD_TESTS)
        enable_testing()
        add_executable(hairtos_tests
            kernel/src/hr_list.c
            kernel/src/hr_scheduler.c
            kernel/src/hr_wait.c
            kernel/src/hr_timeout.c
            kernel/src/hr_task.c
            kernel/src/hr_kernel.c
            kernel/src/hr_time.c
            kernel/src/hr_queue.c
            kernel/src/hr_semaphore.c
            kernel/src/hr_mutex.c
            kernel/src/hr_timer.c
            kernel/src/hr_context.c
            kernel/src/hr_diagnostics.c
            haievent/src/he_event.c
            haievent/src/he_state_machine.c
            haievent/src/he_active.c
            haievent/src/he_time_event.c
            haievent/src/he_pubsub.c
            labs/memory-allocator/src/hr_heap_lab.c
            labs/memory-allocator/src/hr_pool_lab.c
            benchmarks/kernel/src/hr_benchmark_stats.c
            tests/stress/scheduler_stress_core.c
            ${HAIRTOS_TARGET_PORT_STACK_C}
            tests/mocks/mock_port.c
            tests/host/test_main.c
            tests/host/test_list.c
            tests/host/test_ready_queue.c
            tests/host/test_wait_list.c
            tests/host/test_timeout.c
            tests/host/test_port_stack.c
            tests/host/test_task.c
            tests/host/test_kernel_start.c
            tests/host/test_scheduler_policy.c
            tests/host/test_queue.c
            tests/host/test_semaphore.c
            tests/host/test_mutex.c
            tests/host/test_timer.c
            tests/host/test_haievent.c
            tests/host/test_benchmark.c
            tests/host/test_diagnostics.c
            tests/stress/test_scheduler_stress.c
            labs/memory-allocator/tests/test_heap_lab.c)
        target_include_directories(hairtos_tests PRIVATE
            ${HAIRTOS_PUBLIC_INCLUDES}
            ${HAIRTOS_INTERNAL_INCLUDES}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/host
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/stress
            ${CMAKE_CURRENT_SOURCE_DIR}/labs/memory-allocator/include
            ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/kernel/include)
        target_compile_definitions(hairtos_tests PRIVATE
            ${HAIRTOS_TARGET_COMPILE_DEFINITIONS}
            HR_CFG_ENABLE_DIAGNOSTICS=1
            HR_CFG_ENABLE_RUNTIME_STATS=1)
        hairtos_apply_host_warnings(hairtos_tests)
        add_test(NAME hairtos_host_tests COMMAND hairtos_tests)
        set_tests_properties(hairtos_host_tests PROPERTIES
            ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1")
    else()
        hairtos_host_example_config(
            "${HAIRTOS_EXAMPLE}"
            HAIRTOS_HOST_SOURCES
            HAIRTOS_HOST_NEEDS_INTERNAL
            HAIRTOS_HOST_EXTRA_INCLUDES)
        add_executable(hairtos_host ${HAIRTOS_HOST_SOURCES})
        set_target_properties(hairtos_host PROPERTIES OUTPUT_NAME "${HAIRTOS_EXAMPLE}")
        target_include_directories(hairtos_host PRIVATE
            ${HAIRTOS_PUBLIC_INCLUDES}
            ${HAIRTOS_HOST_EXTRA_INCLUDES})
        if(HAIRTOS_HOST_NEEDS_INTERNAL)
            target_include_directories(hairtos_host PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/kernel/internal)
        endif()
        target_compile_definitions(hairtos_host PRIVATE
            ${HAIRTOS_TARGET_COMPILE_DEFINITIONS}
            HR_CFG_ENABLE_DIAGNOSTICS=1
            HR_CFG_ENABLE_RUNTIME_STATS=1)
        hairtos_apply_host_warnings(hairtos_host)
    endif()
endif()
