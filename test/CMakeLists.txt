cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD          14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

project(KeyGenDistributedTests)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(KeyGenDistributedTests
    KeyGenTests.cpp
    MetadataValidityTests.cpp
)

target_include_directories(KeyGenDistributedTests PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    "${CMAKE_CURRENT_LIST_DIR}/../lib/QryptSecurity/include"
)

target_link_libraries(KeyGenDistributedTests PRIVATE
    Threads::Threads
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    "${CMAKE_CURRENT_LIST_DIR}/../lib/QryptSecurity/lib/libQryptSecurity.so"
)