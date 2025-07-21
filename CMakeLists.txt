cmake_minimum_required(VERSION 3.10)
project(PGW LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
find_package(Threads REQUIRED)

# nlohmann/json
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(json)

# spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# cpp-httplib
FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG        v0.14.1
)
FetchContent_MakeAvailable(httplib)

# googletest
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/server
        ${CMAKE_CURRENT_SOURCE_DIR}/client
        ${CMAKE_CURRENT_SOURCE_DIR}/common
)

add_library(pgw_core
        common/bcd_utils.cpp
        server/logger.cpp
        server/config.cpp
        server/cdr_logger.cpp
        server/session_manager.cpp
        server/pgw_server.cpp
)

target_include_directories(pgw_core
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/server
)

target_link_libraries(pgw_core
        PUBLIC
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        Threads::Threads
        httplib::httplib
)

# Server

add_executable(pgw_server server/main.cpp)

target_link_libraries(pgw_server
        PRIVATE
        pgw_core
        httplib::httplib
)

# Client

add_executable(pgw_client client/main.cpp)

target_include_directories(pgw_client
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/client
        ${CMAKE_CURRENT_SOURCE_DIR}/server
)

target_link_libraries(pgw_client
        PRIVATE
        pgw_core
        nlohmann_json::nlohmann_json
)

# Тесты

enable_testing()

# Config
add_executable(test_config tests/test_config.cpp)
target_link_libraries(test_config
        PRIVATE
        pgw_core
        gtest
        gtest_main
)
gtest_discover_tests(test_config)

# SessionManager
add_executable(test_session tests/test_session.cpp)
target_link_libraries(test_session
        PRIVATE
        pgw_core
        gtest
        gtest_main
)
gtest_discover_tests(test_session)

# BCD Conversion
add_executable(test_bcd tests/test_bcd.cpp)
target_link_libraries(test_bcd
        PRIVATE
        pgw_core
        gtest
        gtest_main
)
gtest_discover_tests(test_bcd)

# CDR Logger
add_executable(test_cdr tests/test_cdr.cpp)
target_link_libraries(test_cdr
        PRIVATE
        pgw_core
        gtest
        gtest_main
)
gtest_discover_tests(test_cdr)
