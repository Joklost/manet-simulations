add_executable(lmv lmv.cpp)

target_link_libraries(lmv PUBLIC reachi2)
target_include_directories(lmv PUBLIC ${PROJECT_SOURCE_DIR}/linkmodel_verification)