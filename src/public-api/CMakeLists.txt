######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
set(PUBLIC_API_SRC ummap.cpp)

######################################################
add_library(ummap-public-api OBJECT ${PUBLIC_API_SRC})
SET_TARGET_PROPERTIES(ummap-public-api PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
#if (ENABLE_TESTS)
#	add_subdirectory(tests)
#endif (ENABLE_TESTS)
