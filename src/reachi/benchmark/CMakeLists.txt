add_executable(reachi_benchmark cholesky_benchmark.cpp ../src/datagen.h ../src/datagen.cpp)

target_link_libraries(reachi_benchmark PUBLIC reachi)
target_include_directories(reachi_benchmark PUBLIC ${PROJECT_SOURCE_DIR}/benchmark)