
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD          14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS        OFF) 

project(KeyGenDistributed_project)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")

    # Qrypt Security is built on Windows using the multi-threaded dll runtime library
    # Our executable must use the same runtime library.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    # Enable -fPIC
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

endif()

add_executable(KeyGenDistributed src/main.cpp)

# Pull in Qrypt Security header files
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(KeyGenDistributed PUBLIC MACOS_FRAMEWORK)
    target_include_directories(KeyGenDistributed PUBLIC
        "$ENV{HOME}/Library/Frameworks/QryptSecurity.framework/Headers"
    )
else()
    target_include_directories(KeyGenDistributed PUBLIC
        "lib/QryptSecurity/include"
    )
endif()

# Link against QryptSecurity dynamic library
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(KeyGenDistributed
        "${CMAKE_CURRENT_LIST_DIR}/lib/QryptSecurity/lib/QryptSecurity.lib"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(KeyGenDistributed
       "$ENV{HOME}/Library/Frameworks/QryptSecurity.framework/QryptSecurity"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(KeyGenDistributed
        "${CMAKE_CURRENT_LIST_DIR}/lib/QryptSecurity/lib/libQryptSecurity.so"
    )
endif()

# If on Windows, copy QryptSecurity.dll to the directory where the executable is built
# so that the executable can find it.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET KeyGenDistributed POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_LIST_DIR}/lib/QryptSecurity/bin/QryptSecurity.dll
        $<TARGET_FILE_DIR:KeyGenDistributed>)
endif()
