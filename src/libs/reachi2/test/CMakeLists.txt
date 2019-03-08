add_executable(reachi2_test main.cpp test.cpp)

target_link_libraries(reachi2_test PUBLIC reachi2 Catch2)
target_include_directories(reachi2_test PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestReachi2 COMMAND test_reachi2)