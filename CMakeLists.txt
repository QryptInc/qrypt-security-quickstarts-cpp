
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD          17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS        OFF) 

project(QryptDemo_project)
set(QRYPTDEMO_TARGET "qrypt")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    # Qrypt Security is built on Windows using the multi-threaded dll runtime library
    # Our executable must use the same runtime library.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    # Enable -fPIC
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

endif()

add_executable(${QRYPTDEMO_TARGET}
    src/cli.cpp
    src/common.cpp
    src/keygen.cpp
    src/encrypt.cpp
)

# Pull in Qrypt Security header files
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(${QRYPTDEMO_TARGET} PUBLIC MACOS_FRAMEWORK)
    target_include_directories(${QRYPTDEMO_TARGET} PUBLIC
        "$ENV{HOME}/Library/Frameworks/QryptSecurity.framework/Headers"
    )
else()
    target_include_directories(${QRYPTDEMO_TARGET} PUBLIC
        "QryptSecurity/include"
    )
endif()

# Link against QryptSecurity dynamic library
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(${QRYPTDEMO_TARGET}
        "${CMAKE_CURRENT_LIST_DIR}/QryptSecurity/lib/QryptSecurity.lib"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(${QRYPTDEMO_TARGET}
       "$ENV{HOME}/Library/Frameworks/QryptSecurity.framework/QryptSecurity"
    )
else()
    target_link_libraries(${QRYPTDEMO_TARGET}
        "${CMAKE_CURRENT_LIST_DIR}/QryptSecurity/lib/libQryptSecurity.so"
        "crypto"
    )
endif()

# If on Windows, copy QryptSecurity.dll to the directory where the executable is built
# so that the executable can find it.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET ${QRYPTDEMO_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/QryptSecurity/bin/QryptSecurity.dll
        $<TARGET_FILE_DIR:${QRYPTDEMO_TARGET}>)
endif()
