######################################################
#  PROJECT  : ummap-io-v2                            #
#  LICENSE  : Apache 2.0                             #
#  COPYRIGHT: 2020-2021 Bull SAS All rights reserved #
######################################################

######################################################
set(COMMON_SRC ListElement.cpp HumanUnits.cpp)

######################################################
add_library(ummap-common OBJECT ${COMMON_SRC})
SET_TARGET_PROPERTIES(ummap-common PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
