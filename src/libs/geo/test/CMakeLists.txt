
add_executable(geo_test main.cpp test.cpp)

target_link_libraries(geo_test PUBLIC geo Catch2)
target_include_directories(geo_test PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestLinkAiders COMMAND geo_test)