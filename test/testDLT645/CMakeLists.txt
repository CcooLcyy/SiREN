aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_DLT645_SRC)

add_executable(test_dlt645 ${TEST_DLT645_SRC})
target_link_libraries(test_dlt645 
    siren_dlt645
    GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
)

gtest_discover_tests(test_dlt645)