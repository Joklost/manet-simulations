
add_executable(test_reachi main.cpp test.cpp)

target_link_libraries(test_reachi PUBLIC reachi Catch2)
target_include_directories(test_reachi PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestReachi COMMAND test_reachi)