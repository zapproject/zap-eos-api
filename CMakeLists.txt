cmake_minimum_required(VERSION 3.5)
project(zap VERSION 1.0.0)

# if no wasm root is given use default path
if(EOSIO_WASMSDK_ROOT STREQUAL "" OR NOT EOSIO_WASMSDK_ROOT)
   set(EOSIO_WASMSDK_ROOT /usr/local/eosio.wasmsdk)
endif()

set(BUILD_DIR build/main)
set(SRC_DIR src)

# Set a normal, cache, or environment variable to a given value
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_EXTENSIONS ON )
set( CXX_STANDARD_REQUIRED ON)
set( CLI_CLIENT_EXECUTABLE_NAME cleos )
set( GUI_CLIENT_EXECUTABLE_NAME eosio )

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

# executables output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

# append the path to the module to include
list(APPEND CMAKE_MODULE_PATH ${EOSIO_WASMSDK_ROOT}/lib/cmake)

# include the toolchain cmake
include(${EOSIO_WASMSDK_ROOT}/lib/cmake/EosioWasmToolchain.cmake)

add_executable(main.wast ${SRC_DIR}/main.cpp)
