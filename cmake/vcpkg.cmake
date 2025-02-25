#[========================================================[
Configure for vcpkg
#]========================================================]
message(STATUS "=== begin vcpkg.cmake ==========================================================>")

if(ENABLE_VCPKG)
    STRING(LENGTH $ENV{VCPKG_ROOT} envVcpkgRootLength)
    if(${envVcpkgRootLength} EQUAL 0)
        message(FATAL_ERROR "environment var <VCPKG_ROOT> is empty, maybe you should set the var first.")
    endif()
    message(STATUS "environment var <VCPKG_ROOT>: $ENV{VCPKG_ROOT}")

    execute_process(
        COMMAND ${CMAKE_C_COMPILER} -dumpmachine
        RESULT_VARIABLE result
        OUTPUT_VARIABLE target_arch
        ERROR_VARIABLE error
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "build target: ${target_arch}")
    if(${target_arch} STREQUAL "x86_64-linux-gnu") 
        set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-linux" CACHE STRING "find vcpkg lib in x86")
    elseif(${target_arch} STREQUAL "aarch64-linux-gnu")
        # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--dynamic-linker=/mnt/MskBaseAdv/lib/ld-linux-aarch64.so.1")
        set(CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/arm64-linux-release" CACHE STRING "find vcpkg lib in arm64")
    endif()
    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    message(STATUS "<== end vcpkg.cmake =============================================================")
endif()