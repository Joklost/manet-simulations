add_executable(cholesky_benchmark cholesky.cpp)

target_link_libraries(cholesky_benchmark PUBLIC reachi)
target_include_directories(cholesky_benchmark PUBLIC ${PROJECT_SOURCE_DIR}/benchmark)

add_executable(clustering_benchmark clustering.cpp)

target_link_libraries(clustering_benchmark PUBLIC reachi)
target_include_directories(clustering_benchmark PUBLIC ${PROJECT_SOURCE_DIR}/benchmark)