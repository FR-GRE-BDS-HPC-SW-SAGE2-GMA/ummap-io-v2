######################################################
#            PROJECT  : cdeps                        #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
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
#if (ENABLE_TESTS)
#	add_subdirectory(tests)
#endif()
