
# Custom test target to run the googletest tests
add_custom_target(check)
add_custom_command(
    TARGET check
    POST_BUILD
    COMMENT "All tests were successful!" VERBATIM
)

# Custom test target to just build the googletest tests
add_custom_target(build_check)
add_custom_command(
    TARGET build_check
    POST_BUILD
    COMMENT "All test builds were successful!" VERBATIM
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stamps)

# will compile and run ${test_target}.cpp
# and add all further arguments as dependencies
macro(generic_run_test test_target test_file
      driver register_target register_build_target kind_name nr_pes)

    # The test executable itself
    add_executable(${test_target}_testrunner
        EXCLUDE_FROM_ALL
        ${driver}
        ${test_file}
    )
    target_link_libraries(${test_target}_testrunner
        gtest
        dpt_mpi
        ${ALL_LIBRARIES}
        ${TEST_TARGET_DEPS}
    )

  target_include_directories(${test_target}_testrunner PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dpt/>
    $<INSTALL_INTERFACE:${PROJECT_SOURCE_DIR}/dpt/>
  )

    if(${nr_pes} GREATER 0)
        # Runs the test and generates a stamp file on success.
        add_custom_command(
            OUTPUT
                stamps/${test_target}_testrunner.stamp
            DEPENDS
                ${test_target}_testrunner
            COMMAND
                mpirun -np ${nr_pes} ./${test_target}_testrunner
            COMMAND
                cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/stamps/${test_target}_testrunner.stamp
            WORKING_DIRECTORY
                "${CMAKE_BINARY_DIR}/tests"
            COMMENT
                "Running ${kind_name} ${test_target} ..."
            VERBATIM
        )
    else(${nr_pes} GREATER 0)
        # Runs the test and generates a stamp file on success.
        add_custom_command(
            OUTPUT
                stamps/${test_target}_testrunner.stamp
            DEPENDS
                ${test_target}_testrunner
            COMMAND
                ${test_target}_testrunner
            COMMAND
                cmake -E touch ${CMAKE_CURRENT_BINARY_DIR}/stamps/${test_target}_testrunner.stamp
            WORKING_DIRECTORY
                "${CMAKE_BINARY_DIR}/tests"
            COMMENT
                "Running ${kind_name} ${test_target} ..."
            VERBATIM
        )
    endif(${nr_pes} GREATER 0)


    # The test target. Depends on the stamp file to ensure the
    # test is only run if the source changed
    add_custom_target(
        ${test_target}
        DEPENDS
            stamps/${test_target}_testrunner.stamp
    )

    # Hook into check target
    add_custom_command(
        TARGET ${register_target}
        PRE_BUILD
        COMMAND cmake --build . --target ${test_target}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "${kind_name} ${test_target}" VERBATIM
    )

    # Hook into build_check target
    add_custom_command(
        TARGET ${register_build_target}
        PRE_BUILD
        COMMAND cmake --build . --target ${test_target}_testrunner
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Building ${kind_name} ${test_target}" VERBATIM
    )

    # Ensure binary deps of the testrunner are compiled first
    foreach(bin_dep ${TEST_TARGET_BIN_DEPS})
        add_custom_command(
            TARGET ${test_target}_testrunner
            PRE_BUILD
            COMMAND cmake --build . --target ${bin_dep}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
    endforeach(bin_dep)
endmacro()

macro(run_test test_target)
string(REPLACE "/" "_" test_target_replaced "${test_target}")
generic_run_test(
    ${test_target_replaced}
    "${test_target}.cpp"
    "${CMAKE_SOURCE_DIR}/tests/test_driver.cpp"
    check
    build_check
    "Test"
    0
    ${ARGN}
)
endmacro()

macro(run_distributed_test test_target nr_pes)
string(REPLACE "/" "_" test_target_replaced "${test_target}_proc_${nr_pes}")
generic_run_test(
    ${test_target_replaced}
    "${test_target}.cpp"
    "${CMAKE_SOURCE_DIR}/tests/test_driver_distributed.cpp"
    check
    build_check
    "Test"
    ${nr_pes}
    ${ARGN}
)
endmacro()
