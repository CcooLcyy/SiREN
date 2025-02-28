find_package(boost_uuid CONFIG REQUIRED)
find_package(boost_beast REQUIRED CONFIG)
find_package(boost_asio CONFIG REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} utility_source)

add_library(siren_utility SHARED ${utility_source})

add_library(SiREN::utility ALIAS siren_utility)

target_include_directories(siren_utility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(siren_utility PUBLIC
    Boost::uuid
    Boost::beast
    Boost::asio
)
add_executable(a Restful.cc)
target_link_libraries(a PUBLIC
    Boost::uuid
    Boost::beast
    Boost::asio
)