aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}
    SIREN_LOGGER_SRC
)

add_library(siren_logger
    ${SIREN_LOGGER_SRC}
)

target_include_directories(siren_logger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(siren_logger PUBLIC
    Boost::log
)