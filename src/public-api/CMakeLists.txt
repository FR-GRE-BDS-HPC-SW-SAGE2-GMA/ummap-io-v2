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

if(MERO_FOUND)
	set(CLOVIS_API_SRC clovis_api.c)
	add_library(clovis_api OBJECT ${CLOVIS_API_SRC})
	SET_TARGET_PROPERTIES(clovis_api PROPERTIES POSITION_INDEPENDENT_CODE ON)
	SET_TARGET_PROPERTIES(clovis_api PROPERTIES COMPILE_FLAGS "${MERO_COMPILE_FLAGS}")

	SET_TARGET_PROPERTIES(ummap-public-api PROPERTIES COMPILE_FLAGS "${MERO_COMPILE_FLAGS}")

	install(FILES clovis_api.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif(MERO_FOUND)

######################################################
install(FILES ummap.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
