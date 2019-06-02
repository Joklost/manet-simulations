project(lmac VERSION 1.0)

add_executable(lmac src/lmac.cpp src/models.h)
target_include_directories(lmac PRIVATE ${MPI_CXX_INCLUDE_PATH})
target_compile_options(lmac PRIVATE ${MPI_CXX_COMPILE_FLAGS})
target_link_libraries(lmac ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS} mpilib pthread)
