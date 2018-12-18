set(JSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/json/single_include/nlohmann PARENT_SCOPE)
set(CATCH_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Catch2/single_include/catch2 PARENT_SCOPE)

set(BUILD_CPR_TESTS OFF CACHE BOOL "Don't build cpr tests")

find_package(CURL)
if(CURL_FOUND)
    set(USE_SYSTEM_CURL ON CACHE BOOL "Use the system curl for faster builds")
endif()

find_package(OpenSSL)
if(OPENSSL_FOUND)
    set(CMAKE_USE_OPENSSL ON CACHE BOOL "Use OPENSSL for cpr")
else()
    set(CMAKE_USE_OPENSSL OFF CACHE BOOL "OPENSSL not found")
endif()
add_subdirectory(cpr)

add_subdirectory(spdlog)

add_subdirectory(mpilib)
add_subdirectory(reachi)
