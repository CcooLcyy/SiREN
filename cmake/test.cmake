#[=============================================================================[
# usage: 
1. First turn on the switch of 'ENABLE_TEST', which can help you to include the 
module of test.
2. set the test framework you want to use, you can choose the framework from
'GTest' to 'BoostTest'.
3. After you choose the test framework, you have to find the path of test source
directory. set the variable 'TEST_SOURCE_DIR'.
#]=============================================================================]

message(STATUS "=== begin test.cmake ==========================================================>")
if(ENABLE_TEST) 
    set(notice_message "set a unit test framework from <GTest, BoostTest> to var TEST_FRAMEWORK")
    message(STATUS ${notice_message})
    if(TEST_FRAMEWORK)
        if(TEST_FRAMEWORK STREQUAL "GTest")
            message(STATUS "unit test framework: ${TEST_FRAMEWORK}")
            find_package(GTest CONFIG REQUIRED)
        elseif(TEST_FRAMEWORK STREQUAL "BoostTest")
            message(STATUS "unit test framework: ${TEST_FRAMEWORK}")
            message(FATAL_ERROR "We still not config Boost Test yet, it's will coming soom.
            You can modify this lib for your utlity, also pull request are welcomed.")
        endif()
    else()
        message(FATAL_ERROR "NOT choose a unit test framework")
    endif()
    
    message(STATUS "Testing enabled")
    set(TEST_SOURCE_DIR CACHE PATH "Path to unit test source dir")
    aux_source_directory(${CMAKE_SOURCE_DIR}/src/test test_src_dir)
    foreach(test_src_file ${test_src_dir})
        get_filename_component(test_name ${test_src_file} NAME_WE)
        message(STATUS "unit test file: ${test_src_file}")
        add_executable(test${test_name} ${test_src_file})
        target_compile_options(test${test_name} PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_options(test${test_name} PRIVATE --coverage -lgcov)
        if(${TEST_FRAMEWORK} STREQUAL "GTest")
            target_link_libraries(test${test_name} PUBLIC GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
            add_test(NAME test${test_name} COMMAND test${test_name})
        elseif(${TEST_FRAMEWORK} STREQUAL "BoostTest")
        endif()
    endforeach()
    
    # coverage report support
    if(ENABLE_COVERAGE)
        message(STATUS "Coverage enabled")

        add_custom_target(coverage
            COMMAND rm -rf ${CMAKE_SOURCE_DIR}/coverage-report
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_SOURCE_DIR}/build --config Debug --target all -j 18 --
            COMMAND ${CMAKE_CTEST_COMMAND} --tests-dir ${CMAKE_SOURCE_DIR}/build/cmake
            # COMMAND cd ${CMAKE_SOURCE_DIR}
            COMMAND geninfo . -o coverage.info --no-external --keep-going
            # COMMAND lcov --capture --directory . --output-file coverage.info --no-external
            # COMMAND lcov --capture --directory . --output-file coverage.info --no-external --ignore-errors mismatch
            # COMMAND lcov --remove coverage.info '/usr/*' '/data/3rdlibs/*' --output-file coverage_filtered.info
            # COMMAND genhtml ${CMAKE_SOURCE_DIR}/build/coverage_filtered.info --output-directory ${CMAKE_SOURCE_DIR}/coverage-report
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        message(STATUS "Coverage disabled")
    endif()
endif()
message(STATUS "<== end test.cmake =============================================================")