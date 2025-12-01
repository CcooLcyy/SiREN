aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}
    SIREN_LOGGER_SRC
)


if(${SIREN_BUILD_TYPE} STREQUAL "SHARED")
    add_library(siren_logger SHARED
        ${SIREN_LOGGER_SRC}
    )
elseif(${SIREN_BUILD_TYPE} STREQUAL "STATIC")
    add_library(siren_logger STATIC
        ${SIREN_LOGGER_SRC}
    )
endif()

target_include_directories(siren_logger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(siren_logger PUBLIC
    Boost::log
)