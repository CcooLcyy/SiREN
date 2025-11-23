aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} DLT645_SRC)

add_library(siren_dlt645 SHARED ${DLT645_SRC})

target_include_directories(siren_dlt645 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)