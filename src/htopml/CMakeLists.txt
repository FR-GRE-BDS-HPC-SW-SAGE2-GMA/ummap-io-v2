######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
set(HTOPML_SRC HtopmlMappings.cpp)

######################################################
add_definitions(-DHTOPML_PLUGIN_WWW="${CMAKE_CURRENT_SOURCE_DIR}/www/")

######################################################
add_library(ummap-htopml OBJECT ${HTOPML_SRC})
SET_TARGET_PROPERTIES(ummap-htopml PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
#if (ENABLE_TESTS)
#	add_subdirectory(tests)
#endif (ENABLE_TESTS)
