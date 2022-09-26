cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

project(bmc VERSION 0.1.0)

include(CTest)
enable_testing()
SET(CMAKE_BUILD_TYPE "Debug")

set(FRONTENDS_DIR "${CMAKE_SOURCE_DIR}/frontends")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/deps
					${PROJECT_SOURCE_DIR}/utils
					${PROJECT_SOURCE_DIR}/deps/btor2tools/src
                    )

link_directories(${PROJECT_SOURCE_DIR}/deps/btor2tools/build/lib
				 ${PROJECT_SOURCE_DIR}/deps/smt-switch/build
				 ${PROJECT_SOURCE_DIR}/deps/smt-switch/build/btor
				 ${PROJECT_SOURCE_DIR}/deps/smt-switch/build/cvc5
				 ${PROJECT_SOURCE_DIR}/deps/smt-switch/build/z3
				 ${PROJECT_SOURCE_DIR}/deps/smt-switch/build/msat
				 ${PROJECT_SOURCE_DIR}/deps/smt-switch/build/yices2
				 )


add_executable(bmc main.cpp
                    frontends/btor2_encoder.cpp
					utils/logger.cpp
                    trans/ts.cpp
                    trans/unroller.cpp
					printer/vcd_printer.cpp
                    bmc/bmc.cpp)

target_link_libraries(bmc   libsmt-switch-btor.a 
							libsmt-switch-cvc5.a libsmt-switch-z3.a
							libsmt-switch-msat.a
							libsmt-switch-yices2.a
							libsmt-switch.a
							libbtor2parser.a
							libpthread.so
							-lgmp -lgmpxx) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
