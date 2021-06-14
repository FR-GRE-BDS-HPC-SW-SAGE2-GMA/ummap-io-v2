######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
set(DRIVERS_SRC DummyDriver.cpp
                MemoryDriver.cpp
                FDDriver.cpp
                MmapDriver.cpp
                CDriver.cpp)

######################################################
if (MERO_FOUND OR MOTR_FOUND)
	list(APPEND DRIVERS_SRC ClovisDriver.cpp)
endif(MERO_FOUND OR MOTR_FOUND)

######################################################
if (IOCCLIENT_FOUND)
	list(APPEND DRIVERS_SRC IocDriver.cpp)
endif(IOCCLIENT_FOUND)

######################################################
add_library(ummap-drivers OBJECT ${DRIVERS_SRC})
SET_TARGET_PROPERTIES(ummap-drivers PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
if (MERO_FOUND)
	SET_TARGET_PROPERTIES(ummap-drivers PROPERTIES COMPILE_FLAGS "${MERO_COMPILE_FLAGS}")
endif(MERO_FOUND)
if (MOTR_FOUND)
	SET_TARGET_PROPERTIES(ummap-drivers PROPERTIES COMPILE_FLAGS "${MOTR_COMPILE_FLAGS}")
endif(MOTR_FOUND)

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
