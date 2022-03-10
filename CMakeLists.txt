cmake_minimum_required(VERSION 3.0.0)
project(bmc VERSION 0.1.0)

include(CTest)
enable_testing()

set(SMT_SWITCH_DIR "${CMAKE_SOURCE_DIR}/deps/smt-switch")

add_executable(bmc ${SMT_SWITCH_DIR}/test_smt_switch.cpp)
target_link_libraries(bmc libsmt-switch-z3.so) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
