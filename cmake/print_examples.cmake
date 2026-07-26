include("${CMAKE_CURRENT_LIST_DIR}/hairtos_examples.cmake")

message("Host only:")
foreach(example IN LISTS HAIRTOS_HOST_ONLY_EXAMPLES)
    message("  ${example}")
endforeach()

message("Target only:")
foreach(example IN LISTS HAIRTOS_TARGET_EXAMPLES)
    list(FIND HAIRTOS_DUAL_EXAMPLES "${example}" dual_index)
    if(dual_index EQUAL -1)
        message("  ${example}")
    endif()
endforeach()

message("Host + target:")
foreach(example IN LISTS HAIRTOS_DUAL_EXAMPLES)
    message("  ${example}")
endforeach()
