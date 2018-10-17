
add_executable(reachi_test main.cpp test.cpp)

target_link_libraries(reachi_test PUBLIC reachi Catch)
target_include_directories(reachi_test PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestReachi COMMAND reachi_test)