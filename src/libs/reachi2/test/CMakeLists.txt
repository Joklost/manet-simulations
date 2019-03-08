add_executable(test_reachi2 main.cpp test.cpp)

target_link_libraries(test_reachi2 PUBLIC reachi2 Catch2)
target_include_directories(test_reachi2 PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestReachi2 COMMAND test_reachi2)