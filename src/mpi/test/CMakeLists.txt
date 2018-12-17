
add_executable(mpi_test main.cpp test.cpp)

target_link_libraries(mpi_test PUBLIC mpi)
target_include_directories(mpi_test PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestMPI COMMAND mpi_test)