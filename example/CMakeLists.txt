add_executable(sample main2.cpp main.cpp main3.cpp)

target_link_libraries(sample queue_race boost_filesystem boost_system)
#target_link_libraries(sample profiler)

set_target_properties(sample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})