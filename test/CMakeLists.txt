cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_library(SDKTests_Interface INTERFACE)
target_include_directories(SDKTests_Interface INTERFACE
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    "${CMAKE_CURRENT_LIST_DIR}/../QryptSecurity/include"
)

add_library(SDKTests OBJECT
    KeyGenTests.cpp
)
target_link_libraries(SDKTests PRIVATE
    SDKTests_Interface
)

set(SDKTest_OBJS
    SDKTests_Interface
    SDKTests
    Threads::Threads
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    PARENT_SCOPE
)