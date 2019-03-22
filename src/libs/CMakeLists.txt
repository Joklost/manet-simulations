set(CATCH_BUILD_TESTING OFF CACHE BOOL "Don't build Catch2 tests")
set(EIGEN_SPLIT_LARGE_TESTS OFF CACHE BOOL "Don't split Eigen tests")

add_subdirectory(Catch2)

add_subdirectory(common)
add_subdirectory(geo)

if(INSTALL_HTTP)
    add_subdirectory(http)
endif()

add_subdirectory(eigen)
add_subdirectory(reachi)
add_subdirectory(reachi2)

add_subdirectory(lm)

add_subdirectory(mpilib)
