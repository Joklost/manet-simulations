
add_executable(test_geo main.cpp test.cpp)

target_link_libraries(test_geo PUBLIC geo Catch2)
target_include_directories(test_geo PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestLinkAiders COMMAND test_geo)