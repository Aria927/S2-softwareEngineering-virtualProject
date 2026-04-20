cmake_minimum_required(VERSION 3.20)

project(PasswordManagerBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(password_manager_backend
    src/AdminService.cpp
    src/AuditService.cpp
    src/AuthService.cpp
    src/CryptoService.cpp
    src/MfaService.cpp
    src/SearchService.cpp
    src/VaultService.cpp
)

target_include_directories(password_manager_backend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(password_manager_backend
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

find_path(MYSQL_INCLUDE_DIR
    NAMES mysql.h
    PATHS
        "C:/Program Files/MySQL/MySQL Server 8.0/include"
)

find_library(MYSQL_LIBRARY
    NAMES libmysql mysqlclient
    PATHS
        "C:/Program Files/MySQL/MySQL Server 8.0/lib"
)

if (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    target_sources(password_manager_backend PRIVATE src/MySqlRepositories.cpp)
    target_include_directories(password_manager_backend PRIVATE ${MYSQL_INCLUDE_DIR})
    target_link_libraries(password_manager_backend PRIVATE ${MYSQL_LIBRARY})
endif()

add_executable(password_manager_backend_tests
    tests/backend_tests.cpp
)

target_link_libraries(password_manager_backend_tests
    PRIVATE
        password_manager_backend
)

enable_testing()

add_test(
    NAME password_manager_backend_tests
    COMMAND password_manager_backend_tests
)
