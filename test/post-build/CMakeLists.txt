find_program(H5DIFF_EXECUTABLE h5diff REQUIRED)

# loop over the test cases
foreach(N IN ITEMS 1 2 3)

    # Do it with 1 or 2 threads
    foreach(J IN ITEMS 1 2)

        # Run 1 (fixture setup): run opentrim with J threads, produce out${N}j${J}t1.h5
        add_test(NAME PostBuild_RunSim_${N}_${J}_1
            COMMAND "$<TARGET_FILE:opentrim_exe>"
                -f "${CMAKE_CURRENT_SOURCE_DIR}/in${N}.json"
                -j ${J} -o out${N}j${J}t1
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
        set_tests_properties(PostBuild_RunSim_${N}_${J}_1 PROPERTIES
            FIXTURES_SETUP PostBuildFixture_${N}_${J}
        )
        if(WIN32)
            set_tests_properties(PostBuild_RunSim_${N}_${J}_1 PROPERTIES
                ENVIRONMENT_MODIFICATION
                    "PATH=path_list_prepend:$<TARGET_FILE_DIR:${PROJECT_NAME_LOWERCASE}>;PATH=path_list_prepend:$<TARGET_FILE_DIR:xs_zbl>"
            )
        endif()

        # Run 2 (fixture setup): same config again, produce out${N}j${J}t2.h5
        add_test(NAME PostBuild_RunSim_${N}_${J}_2
            COMMAND "$<TARGET_FILE:opentrim_exe>"
                -f "${CMAKE_CURRENT_SOURCE_DIR}/in${N}.json"
                -j ${J} -o out${N}j${J}t2
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )
        set_tests_properties(PostBuild_RunSim_${N}_${J}_2 PROPERTIES
            FIXTURES_SETUP PostBuildFixture_${N}_${J}
        )
        if(WIN32)
            set_tests_properties(PostBuild_RunSim_${N}_${J}_2 PROPERTIES
                ENVIRONMENT_MODIFICATION
                    "PATH=path_list_prepend:$<TARGET_FILE_DIR:${PROJECT_NAME_LOWERCASE}>;PATH=path_list_prepend:$<TARGET_FILE_DIR:xs_zbl>"
            )
        endif()

        # Compare the two runs against each other using h5diff
        add_test(NAME PostBuild_Compare_${N}_${J}
            COMMAND "${CMAKE_COMMAND}"
                "-DH5DIFF=${H5DIFF_EXECUTABLE}"
                "-DFILE1=${CMAKE_CURRENT_BINARY_DIR}/out${N}j${J}t1.h5"
                "-DFILE2=${CMAKE_CURRENT_BINARY_DIR}/out${N}j${J}t2.h5"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/compare.cmake"
        )
        set_tests_properties(PostBuild_Compare_${N}_${J} PROPERTIES
            FIXTURES_REQUIRED PostBuildFixture_${N}_${J}
        )

        # Fixture cleanup
        add_test(NAME PostBuild_Cleanup_${N}_${J}
            COMMAND "${CMAKE_COMMAND}" -E rm -f
                "${CMAKE_CURRENT_BINARY_DIR}/out${N}j${J}t1.h5"
                "${CMAKE_CURRENT_BINARY_DIR}/out${N}j${J}t2.h5"
        )
        set_tests_properties(PostBuild_Cleanup_${N}_${J} PROPERTIES
            FIXTURES_CLEANUP PostBuildFixture_${N}_${J}
        )

    endforeach()
endforeach()
