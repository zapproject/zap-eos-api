cmake_minimum_required(VERSION 3.5)
project(zap VERSION 0.1.0)

# if no wasm root is given use default path
if(EOSIO_WASMSDK_ROOT STREQUAL "" OR NOT EOSIO_WASMSDK_ROOT)
   set(EOSIO_WASMSDK_ROOT /usr/local/eosio.cdt)
endif()

set(BUILD_DIR build/main)
set(SRC_DIR src)

# executables output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

# append the path receiver the module receiver include
list(APPEND CMAKE_MODULE_PATH ${EOSIO_WASMSDK_ROOT}/lib/cmake)

include_directories(${SRC_DIR})
include_directories(${EOSIO_WASMSDK_ROOT}/include)

include(EosioWasmToolchain)

add_executable(main.wasm ${SRC_DIR}/main.cpp)
