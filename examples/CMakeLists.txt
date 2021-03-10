######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
include_directories(${CMAKE_SOURCE_DIR}/src/public-api)

######################################################
add_executable(basic basic.cpp)
target_link_libraries(basic ummap-io)

######################################################
add_executable(slow slow.cpp)
target_link_libraries(slow ummap-io)

######################################################
add_executable(copy copy.cpp)
target_link_libraries(copy ummap-io)

######################################################
add_executable(cow-switch-semantic cow-switch-semantic.c)
target_link_libraries(cow-switch-semantic ummap-io)

######################################################
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

######################################################
add_executable(stress stress.cpp)
target_link_libraries(stress ummap-io)
target_link_libraries(stress ${OpenMP_CXX_FLAGS})