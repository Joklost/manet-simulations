
add_executable(test_linkaiders main.cpp test.cpp)

target_link_libraries(test_linkaiders PUBLIC linkaiders Catch2)
target_include_directories(test_linkaiders PUBLIC ${PROJECT_SOURCE_DIR}/test)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gpslog.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_test(NAME TestLinkAiders COMMAND test_linkaiders)