cmake_minimum_required(VERSION 3.5)
project(zap VERSION 0.1.0)

find_package(eosio.cdt)

set(BUILD_DIR build)
set(SRC_DIR src)

# executables output directory
set(CMAKE_TOKEN_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}/token)
set(CMAKE_MAIN_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR}/main)

include_directories(${SRC_DIR})

add_contract( main main ${SRC_DIR}/main.cpp )
add_contract( eosio.token eosio.token ${SRC_DIR}/eosio.token.cpp )

add_custom_command(
        TARGET main.wasm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ./main.abi
                ${CMAKE_MAIN_RUNTIME_OUTPUT_DIRECTORY}/main.abi)

add_custom_command(
        TARGET eosio.token.wasm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ./eosio.token.abi
                ${CMAKE_TOKEN_RUNTIME_OUTPUT_DIRECTORY}/eosio.token.abi)


set_target_properties(eosio.token.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_TOKEN_RUNTIME_OUTPUT_DIRECTORY}")

set_target_properties(main.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_MAIN_RUNTIME_OUTPUT_DIRECTORY}")