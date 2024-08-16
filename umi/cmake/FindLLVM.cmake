execute_process(COMMAND which port RESULT_VARIABLE MACPORTS_DETECTED OUTPUT_VARIABLE MACPORTS_PREFIX ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
if (${MACPORTS_DETECTED} EQUAL 0)
    # "/opt/local/bin/port" doesn't have libs, so we get the parent directory
    get_filename_component(MACPORTS_PREFIX ${MACPORTS_PREFIX} DIRECTORY)

    # "/opt/local/bin" doesn't have libs, so we get the parent directory
    get_filename_component(MACPORTS_PREFIX ${MACPORTS_PREFIX} DIRECTORY)

    list(APPEND CMAKE_PREFIX_PATH ${MACPORTS_PREFIX}/libexec/llvm-18/lib/cmake/llvm/)
endif ()