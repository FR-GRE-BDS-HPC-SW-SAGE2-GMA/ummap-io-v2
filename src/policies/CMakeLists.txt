######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
set(POLICIES_SRC FifoPolicy.cpp)

######################################################
add_library(ummap-policies OBJECT ${POLICIES_SRC})
SET_TARGET_PROPERTIES(ummap-policies PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
#if (ENABLE_TESTS)
#	add_subdirectory(tests)
#endif (ENABLE_TESTS)
