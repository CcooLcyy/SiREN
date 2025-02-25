message(STATUS "=== begin config.cmake ==========================================================>")
message(STATUS "printing configure")
message(STATUS "You can modify this configure in the top of cmake file")
#[========================================================[
Configure for vcpkg module.
#]========================================================]
message(STATUS ">>>---------- vcpkg configure")
option(ENABLE_VCPKG "include 3rdlibs by vcpkg" ON)
message(STATUS "ENABLE_VCPKG: ${ENABLE_VCPKG}")
#[========================================================[
Configure for test module.
#]========================================================]
# switch for testing
message(STATUS ">>>---------- testing configure")
option(ENABLE_TEST "Enable Testing" ON)
option(ENABLE_COVERAGE "Enable coverage reporting" ON)
message(STATUS "ENABLE_TEST: ${ENABLE_TEST}")
message(STATUS "ENABLE_COVERAGE: ${ENABLE_COVERAGE}")

# select a unit test framework from <GTest, BoostTest>
set(TEST_FRAMEWORK "GTest" CACHE STRING "choose test framework.")
message(STATUS "TEST_FRAMEWORK: ${TEST_FRAMEWORK}")

message(STATUS "<== end config.cmake =============================================================")