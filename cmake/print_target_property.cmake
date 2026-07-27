
include("${CMAKE_CURRENT_LIST_DIR}/hairtos_targets.cmake")

if(NOT DEFINED HAIRTOS_TARGET OR HAIRTOS_TARGET STREQUAL "")
    set(HAIRTOS_TARGET "${HAIRTOS_DEFAULT_TARGET}")
endif()
if(NOT DEFINED HAIRTOS_PROPERTY OR HAIRTOS_PROPERTY STREQUAL "")
    message(FATAL_ERROR "HAIRTOS_PROPERTY is required")
endif()

hairtos_validate_target("${HAIRTOS_TARGET}")
include("${CMAKE_CURRENT_LIST_DIR}/targets/${HAIRTOS_TARGET}.cmake")

set(_variable "HAIRTOS_TARGET_${HAIRTOS_PROPERTY}")
if(NOT DEFINED ${_variable})
    message(FATAL_ERROR "Target property '${HAIRTOS_PROPERTY}' is not defined")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E echo "${${_variable}}")
