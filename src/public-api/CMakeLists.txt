######################################################
#  PROJECT  : ummap-io-v2                            #
#  LICENSE  : Apache 2.0                             #
#  COPYRIGHT: 2020-2021 Bull SAS All rights reserved #
######################################################

######################################################
set(PUBLIC_API_SRC ummap.cpp)

######################################################
add_library(ummap-public-api OBJECT ${PUBLIC_API_SRC})
SET_TARGET_PROPERTIES(ummap-public-api PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
if(MERO_FOUND)
	SET_TARGET_PROPERTIES(ummap-public-api PROPERTIES COMPILE_FLAGS "${MERO_COMPILE_FLAGS}")
endif(MERO_FOUND)
if(MOTR_FOUND)
	SET_TARGET_PROPERTIES(ummap-public-api PROPERTIES COMPILE_FLAGS "${MOTR_COMPILE_FLAGS}")
endif(MOTR_FOUND)

######################################################
install(FILES ummap.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ummap)

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
