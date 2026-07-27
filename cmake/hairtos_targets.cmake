# Hardware target discovery and manifest validation.
#
# Every *.cmake file under cmake/targets is a selectable target manifest. The
# template uses the .example suffix, so it is intentionally not discovered.

set(HAIRTOS_DEFAULT_TARGET "bluepill_f103c8")

file(GLOB _hairtos_target_manifests
    RELATIVE "${CMAKE_CURRENT_LIST_DIR}/targets"
    "${CMAKE_CURRENT_LIST_DIR}/targets/*.cmake")
set(HAIRTOS_SUPPORTED_TARGETS "")
foreach(_manifest IN LISTS _hairtos_target_manifests)
    get_filename_component(_target "${_manifest}" NAME_WE)
    list(APPEND HAIRTOS_SUPPORTED_TARGETS "${_target}")
endforeach()
list(SORT HAIRTOS_SUPPORTED_TARGETS)

function(hairtos_validate_target target)
    list(FIND HAIRTOS_SUPPORTED_TARGETS "${target}" _target_index)
    if(_target_index EQUAL -1)
        string(JOIN ", " _supported ${HAIRTOS_SUPPORTED_TARGETS})
        message(FATAL_ERROR
            "Unsupported HAIRTOS_TARGET '${target}'. Supported targets: ${_supported}")
    endif()
endfunction()

function(hairtos_validate_target_manifest)
    set(_required_scalar_variables
        HAIRTOS_TARGET_MANIFEST_VERSION
        HAIRTOS_TARGET_NAME
        HAIRTOS_TARGET_DESCRIPTION
        HAIRTOS_TARGET_ARCH
        HAIRTOS_TARGET_SOC
        HAIRTOS_TARGET_BOARD
        HAIRTOS_TARGET_LINKER_SCRIPT
        HAIRTOS_TARGET_OPENOCD_CONFIG
        HAIRTOS_TARGET_OPENOCD_ERASE_COMMAND)

    set(_required_list_variables
        HAIRTOS_TARGET_CPU_FLAGS
        HAIRTOS_TARGET_PUBLIC_INCLUDES
        HAIRTOS_TARGET_PLATFORM_C
        HAIRTOS_TARGET_PLATFORM_ASM
        HAIRTOS_TARGET_PORT_C
        HAIRTOS_TARGET_PORT_STACK_C
        HAIRTOS_TARGET_PORT_ASM
        HAIRTOS_TARGET_KERNEL_TICK_C
        HAIRTOS_TARGET_BAREMETAL_TICK_C
        HAIRTOS_TARGET_FAULT_C
        HAIRTOS_TARGET_FAULT_ASM
        HAIRTOS_TARGET_BENCHMARK_CLOCK_C)

    foreach(_variable IN LISTS _required_scalar_variables _required_list_variables)
        if(NOT DEFINED ${_variable} OR "${${_variable}}" STREQUAL "")
            message(FATAL_ERROR
                "Target manifest '${HAIRTOS_TARGET}' must define ${_variable}")
        endif()
    endforeach()

    if(NOT HAIRTOS_TARGET_MANIFEST_VERSION EQUAL 1)
        message(FATAL_ERROR
            "Target '${HAIRTOS_TARGET}' uses unsupported manifest version "
            "'${HAIRTOS_TARGET_MANIFEST_VERSION}'")
    endif()

    if(NOT HAIRTOS_TARGET_NAME STREQUAL HAIRTOS_TARGET)
        message(FATAL_ERROR
            "Target manifest name '${HAIRTOS_TARGET_NAME}' must match "
            "HAIRTOS_TARGET '${HAIRTOS_TARGET}'")
    endif()

    set(_source_variables
        HAIRTOS_TARGET_PLATFORM_C
        HAIRTOS_TARGET_PLATFORM_ASM
        HAIRTOS_TARGET_PORT_C
        HAIRTOS_TARGET_PORT_STACK_C
        HAIRTOS_TARGET_PORT_ASM
        HAIRTOS_TARGET_KERNEL_TICK_C
        HAIRTOS_TARGET_BAREMETAL_TICK_C
        HAIRTOS_TARGET_FAULT_C
        HAIRTOS_TARGET_FAULT_ASM
        HAIRTOS_TARGET_BENCHMARK_CLOCK_C)

    foreach(_variable IN LISTS _source_variables)
        foreach(_path IN LISTS ${_variable})
            if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_path}")
                message(FATAL_ERROR
                    "Target '${HAIRTOS_TARGET}' references missing file '${_path}'")
            endif()
        endforeach()
    endforeach()

    foreach(_path IN LISTS HAIRTOS_TARGET_PUBLIC_INCLUDES)
        if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${_path}")
            message(FATAL_ERROR
                "Target '${HAIRTOS_TARGET}' references missing include directory '${_path}'")
        endif()
    endforeach()

    foreach(_variable HAIRTOS_TARGET_LINKER_SCRIPT HAIRTOS_TARGET_OPENOCD_CONFIG)
        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${${_variable}}")
            message(FATAL_ERROR
                "Target '${HAIRTOS_TARGET}' references missing file "
                "'${${_variable}}'")
        endif()
    endforeach()
endfunction()
