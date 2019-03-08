
add_executable(linkaiders_test main.cpp test.cpp)

target_link_libraries(linkaiders_test PUBLIC linkaiders Catch2)
target_include_directories(linkaiders_test PUBLIC ${PROJECT_SOURCE_DIR}/test)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gpslog.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_test(NAME TestLinkAiders COMMAND test_linkaiders)