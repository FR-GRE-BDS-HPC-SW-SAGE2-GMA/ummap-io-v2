######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
set(DRIVERS_SRC DummyDriver.cpp
				MemoryDriver.cpp
				FDDriver.cpp)

######################################################
add_library(ummap-drivers OBJECT ${DRIVERS_SRC})
SET_TARGET_PROPERTIES(ummap-drivers PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
