aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} DLT645_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/protobuf/gen DLT645_PROTOBUF_SRC)

execute_process(
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/genProto.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/protobuf
    RESULT_VARIABLE PYTHON_RESULT
    OUTPUT_VARIABLE PYTHON_OUTPUT
    ERROR_VARIABLE PYTHON_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS ${PYTHON_OUTPUT})

add_library(siren_dlt645_shared SHARED 
    ${DLT645_SRC}
    ${DLT645_PROTOBUF_SRC}
)
target_include_directories(siren_dlt645_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/gen
)
target_link_libraries(siren_dlt645_shared PUBLIC 
    protobuf::libprotobuf
)

add_library(siren_dlt645_static STATIC 
    ${DLT645_SRC}
    ${DLT645_PROTOBUF_SRC}
)
target_include_directories(siren_dlt645_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/gen
)
target_link_libraries(siren_dlt645_static PUBLIC 
    protobuf::libprotobuf
)