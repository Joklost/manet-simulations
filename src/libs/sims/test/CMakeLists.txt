
add_executable(test_sims main.cpp test.cpp)

target_link_libraries(test_sims PUBLIC sims Catch2)
target_include_directories(test_sims PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestSims COMMAND test_sims)