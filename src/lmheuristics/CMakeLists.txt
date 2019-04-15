project(lmh VERSION 1.0)

add_executable(lmh src/lmh.cpp)

target_link_libraries(lmh sims2 common geo)
