cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

project(bmc VERSION 0.1.0)

include(CTest)
enable_testing()
SET(CMAKE_BUILD_TYPE "Debug")

set(FRONTENDS_DIR "${CMAKE_SOURCE_DIR}/frontends")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# option(BUILD_STATIC "build static binary" OFF)
if(BUILD_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS -static)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
  set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
  set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
  set(CMAKE_SHARED_LIBRARY_C_FLAGS)         # remove -fPIC
  set(CMAKE_SHARED_LIBRARY_CXX_FLAGS)
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)    # remove -rdynamic
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
  # Maybe this works as well, haven't tried yet.
  # set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
else(BUILD_STATIC)
  # Set RPATH to use for installed targets; append linker search path
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LOFAR_LIBDIR}")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif(BUILD_STATIC)


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
							-lpthread
							-lgmpxx
							-lgmp) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
