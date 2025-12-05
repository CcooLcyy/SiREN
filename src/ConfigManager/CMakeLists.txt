aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SIREN_CONFIGMANAGER_SRC)

add_library(siren_configManager
    ${SIREN_CONFIGMANAGER_SRC}
)

target_include_directories(siren_configManager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(siren_configManager PUBLIC
    Boost::log
)