project(evolutiondex_tests)
cmake_minimum_required(VERSION 3.12)

find_package(eosio)

enable_testing()

configure_file(${CMAKE_SOURCE_DIR}/contracts.hpp.in ${CMAKE_BINARY_DIR}/contracts.hpp)

include_directories(${CMAKE_BINARY_DIR})

file(GLOB UNIT_TESTS "*.cpp" "*.hpp")

add_eosio_test( unit_test ${UNIT_TESTS} )

