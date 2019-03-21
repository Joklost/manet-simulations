set(JSON_BuildTests OFF CACHE BOOL "Don't build json tests")
set(BUILD_CPR_TESTS OFF CACHE BOOL "Don't build cpr tests")
set(CATCH_BUILD_TESTING OFF CACHE BOOL "Don't build Catch2 tests")
set(EIGEN_SPLIT_LARGE_TESTS OFF CACHE BOOL "Don't split Eigen tests")

add_subdirectory(Catch2)

add_subdirectory(common)
add_subdirectory(geo)

if(INSTALL_HTTP)
    find_package(CURL)
    if(CURL_FOUND)
        set(USE_SYSTEM_CURL ON CACHE BOOL "Use the system curl for faster builds")
        set(CURL_CA_PATH_SET NONE)
    endif()

    find_package(OpenSSL)
    if(OPENSSL_FOUND)
        set(CMAKE_USE_OPENSSL ON CACHE BOOL "Use OPENSSL for cpr")
    else()
        set(CMAKE_USE_OPENSSL OFF CACHE BOOL "OPENSSL not found")
    endif()

    add_subdirectory(json)
    add_subdirectory(cpr)
    add_subdirectory(http)
endif()

add_subdirectory(eigen)
add_subdirectory(reachi)
add_subdirectory(reachi2)

add_subdirectory(lm)

add_subdirectory(spdlog)
add_subdirectory(mpilib)
