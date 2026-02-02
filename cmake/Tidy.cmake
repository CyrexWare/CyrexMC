# Check executable exists
if(NOT EXISTS ${CLANG_TIDY_EXECUTABLE})
    find_program(CLANG_TIDY_EXEC_TEMP ${CLANG_TIDY_EXECUTABLE})
    if(CLANG_TIDY_EXEC_TEMP)
        set(CLANG_TIDY_EXECUTABLE ${CLANG_TIDY_EXEC_TEMP})
        unset(CLANG_TIDY_EXEC_TEMP)
    else()
        message(FATAL_ERROR "Unable to find clang-tidy executable: \"${CLANG_TIDY_EXECUTABLE}\"")
    endif()
endif()


# Check executable version
execute_process(
    COMMAND ${CLANG_TIDY_EXECUTABLE} --version
    OUTPUT_VARIABLE CLANG_TIDY_VERSION_OUTPUT
    ERROR_VARIABLE CLANG_TIDY_VERSION_OUTPUT
)
string(REGEX MATCH "version ([0-9]+)" _ "${CLANG_TIDY_VERSION_OUTPUT}")
unset(CLANG_TIDY_VERSION)
if(CMAKE_MATCH_1 GREATER_EQUAL 14)
    message(STATUS "Using clang-tidy version ${CMAKE_MATCH_1}")
else()
    message(FATAL_ERROR "clang-tidy version ${CMAKE_MATCH_1} is too low")
endif()

# Find Python and run-clang-tidy script
find_package(Python 3 REQUIRED)

set(RUN_CLANG_TIDY "${CMAKE_CURRENT_SOURCE_DIR}/run-clang-tidy")
message(STATUS "run-clang-tidy path: ${RUN_CLANG_TIDY}")

# Run
execute_process(COMMAND ${Python_EXECUTABLE} ${RUN_CLANG_TIDY} -clang-tidy-binary ${CLANG_TIDY_EXECUTABLE} -quiet -p ${PROJECT_BINARY_DIR} "^((?!_deps).)*$" RESULTS_VARIABLE EXIT_CODE)
if(NOT EXIT_CODE STREQUAL 0)
    message(FATAL_ERROR "Analysis failed")
endif()


