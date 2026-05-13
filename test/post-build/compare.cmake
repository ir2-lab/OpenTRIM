execute_process(
    COMMAND "${H5DIFF}" -v --relative=1e-9 "${PRODUCED}" "${TEMPLATE}" /tally
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
)
if(result)
    message(FATAL_ERROR "h5diff mismatch in /tally:\n${output}${error}")
endif()
