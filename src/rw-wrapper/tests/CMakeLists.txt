######################################################
#            PROJECT  : cdeps                        #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 10/2018                      #
#            LICENSE  : ????????                     #
######################################################

######################################################
#Use gtest
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

######################################################
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

######################################################
set(TEST_NAMES TestRwWrapper)

######################################################
FOREACH(test_name ${TEST_NAMES})
	add_executable(${test_name} ${test_name}.cpp)
	target_link_libraries(${test_name} $<TARGET_OBJECTS:ummap-io-rw-wrapper-code>)
	target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES} ${GMOCK_BOTH_LIBRARIES} ummap-io dl)
	ummap_add_test(${test_name} ${test_name})
ENDFOREACH(test_name)
