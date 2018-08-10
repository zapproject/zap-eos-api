cmake_minimum_required(VERSION 3.5)
project(zap VERSION 1.0.0)

# if no wasm root is given use default path
if(EOSIO_WASMSDK_ROOT STREQUAL "" OR NOT EOSIO_WASMSDK_ROOT)
   set(EOSIO_WASMSDK_ROOT /usr/local/eosio.wasmsdk)
endif()

set(BUILD_DIR ./build/main)
set(SRC_DIR ./src)

# append the path to the module to include
list(APPEND CMAKE_MODULE_PATH ${EOSIO_WASMSDK_ROOT}/lib/cmake)

#include the toolchain cmake
include(${EOSIO_WASMSDK_ROOT}/lib/cmake/EosioWasmToolchain.cmake)

add_executable( ${BUILD_DIR}/main.wast ${SRC_DIR}/main.cpp )
