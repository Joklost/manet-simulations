
add_executable(test_mpilib main.cpp test.cpp)

target_link_libraries(test_mpilib PUBLIC mpilib Catch2)
target_include_directories(test_mpilib PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestMPI COMMAND test_mpilib)