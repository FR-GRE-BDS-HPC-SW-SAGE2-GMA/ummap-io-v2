######################################################
#            PROJECT  : cdeps                        #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
#Use gtest
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

######################################################
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

######################################################
set(TEST_NAMES TestMapping TestMappingRegistry TestPolicyRegistry TestPolicy TestGlobalHandler)

######################################################
FOREACH(test_name ${TEST_NAMES})
	add_executable(${test_name} ${test_name}.cpp)
	target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES} ${GMOCK_BOTH_LIBRARIES} ${OpenMP_EXE_LINKER_FLAGS} ummap-io)
	ummap_add_test(${test_name} ${test_name})
ENDFOREACH(test_name)
