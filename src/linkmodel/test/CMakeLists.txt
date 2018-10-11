
add_executable(linkmodel_test main.cpp test.cpp)

target_link_libraries(linkmodel_test PUBLIC linkmodel Catch)
target_include_directories(linkmodel_test PUBLIC ${PROJECT_SOURCE_DIR}/test)

add_test(NAME TestLinkmodel COMMAND linkmodel_test)