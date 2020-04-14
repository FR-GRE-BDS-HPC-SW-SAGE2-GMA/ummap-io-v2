######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################

if (MERO_FOUND)
	set(DRIVERS_SRC DummyDriver.cpp
					MemoryDriver.cpp
					FDDriver.cpp
					ClovisDriver.cpp)

	add_library(ummap-drivers OBJECT ${DRIVERS_SRC})
	SET_TARGET_PROPERTIES(ummap-drivers PROPERTIES POSITION_INDEPENDENT_CODE ON)
	SET_TARGET_PROPERTIES(ummap-drivers PROPERTIES COMPILE_FLAGS "${MERO_COMPILE_FLAGS}")

else(MERO_FOUND)
	set(DRIVERS_SRC DummyDriver.cpp
					MemoryDriver.cpp
					FDDriver.cpp)
	add_library(ummap-drivers OBJECT ${DRIVERS_SRC})
	SET_TARGET_PROPERTIES(ummap-drivers PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif(MERO_FOUND)


if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
