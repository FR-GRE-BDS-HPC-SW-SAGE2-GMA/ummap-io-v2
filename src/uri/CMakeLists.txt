######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
set(URI_SRC Uri.cpp UriHandler.cpp Listings.cpp RessourceHandler.cpp)

######################################################
add_library(ummap-uri OBJECT ${URI_SRC})
SET_TARGET_PROPERTIES(ummap-uri PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
