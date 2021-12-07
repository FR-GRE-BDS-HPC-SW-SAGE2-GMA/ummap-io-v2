######################################################
#  PROJECT  : ummap-io-v2                            #
#  LICENSE  : Apache 2.0                             #
#  COPYRIGHT: 2020-2021 Bull SAS All rights reserved #
######################################################

######################################################
set(PORTABILITY_SRC "")
if (WITH_OS STREQUAL "Unix")
	list(APPEND PORTABILITY_SRC UnixOS.cpp)
endif()

######################################################
add_library(ummap-portability OBJECT ${PORTABILITY_SRC})
SET_TARGET_PROPERTIES(ummap-portability PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif()
