cmake_minimum_required(VERSION 3.0.0)
project(bmc VERSION 0.1.0)

include(CTest)
enable_testing()

set(FRONTENDS_DIR "${CMAKE_SOURCE_DIR}/frontends")



include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/deps
                    )

link_directories(${PROJECT_SOURCE_DIR}/deps/btor2parser)

add_executable(bmc main.cpp
                    frontends/btor2_encoder.cpp
                    trans/ts.cpp)

target_link_libraries(bmc  libsmt-switch.so libsmt-switch-z3.so libbtor2parser.so) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
