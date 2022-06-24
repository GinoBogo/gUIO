cmake_minimum_required(VERSION 3.20)

set(COMPILER "gcc")

if(COMPILER STREQUAL "clang")
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        set(CMAKE_C_COMPILER "clang.exe")
        set(CMAKE_CXX_COMPILER "clang++.exe")
    else()
        set(CMAKE_C_COMPILER "clang")
        set(CMAKE_CXX_COMPILER "clang++")
    endif()

    if(${CMAKE_BUILD_TYPE} STREQUAL "debug")
        add_compile_options(-fstandalone-debug)
    endif()
else()
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
        set(CMAKE_C_COMPILER "gcc.exe")
        set(CMAKE_CXX_COMPILER "g++.exe")
    else()
        set(CMAKE_C_COMPILER "gcc")
        set(CMAKE_CXX_COMPILER "g++")
    endif()
endif()

# set(CMAKE_C_EXTENSIONS FALSE)
# set(CMAKE_CXX_EXTENSIONS FALSE)
if(${CMAKE_VERSION} VERSION_LESS_EQUAL "3.20")
    set(CMAKE_C_STANDARD 11)
else()
    set(CMAKE_C_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD 20)

message("================================================================================")
message("    OS: " ${CMAKE_HOST_SYSTEM_NAME})
message("   DIR: " ${CMAKE_SOURCE_DIR})
message("  TYPE: " ${CMAKE_BUILD_TYPE})
message(" CMAKE: " ${CMAKE_VERSION})
message(" C/C++: " ${CMAKE_C_STANDARD}/${CMAKE_CXX_STANDARD})
message("================================================================================")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

project(FIFO)

add_compile_options(
    -fno-omit-frame-pointer
    -pedantic
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-unused-result
    -Wno-unused-variable
    -Wshadow
    -Wsign-conversion
    -Wswitch-default
)

if(COMPILE_LANGUAGE:CXX)
    add_compile_options(-Wold-style-cast)
endif()

add_library(gLIB OBJECT
    "../../lib/GBuffer.cpp"
    "../../lib/GFiFo.cpp"
    "../../lib/GLogger.cpp"
    "../../lib/GMessage.cpp"
    "../../lib/GOptions.cpp"
    "../../lib/GPacket.cpp"
    "../../lib/GUdpClient.cpp"
    "../../lib/GUdpServer.cpp"
)

add_library(gUIO OBJECT
    "../../uio/GFIFOdevice.cpp"
    "../../uio/GMAPdevice.cpp"
    "../../uio/GRegisters.cpp"
    "../../uio/GUIOdevice.cpp"
)

include_directories(
    "../../lib"
    "../../uio"
    "./src"
)

add_executable(_fifo
    "./src/main.cpp"
    "./src/globals.cpp"
    "./src/streams.cpp"
)
target_link_libraries(_fifo pthread gLIB gUIO)
