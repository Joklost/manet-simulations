
add_executable(mpilib_test main.cpp test.cpp)

target_link_libraries(mpilib_test PUBLIC mpilib)
target_include_directories(mpilib_test PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestMPILib COMMAND mpilib_test)