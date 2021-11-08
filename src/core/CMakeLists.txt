######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
set(CORE_SRC Driver.cpp 
			 Mapping.cpp
			 Policy.cpp
			 PolicyQuota.cpp
			 PolicyQuotaLocal.cpp
			 PolicyQuotaInterProc.cpp
			 MappingRegistry.cpp
			 GlobalHandler.cpp
			 PolicyRegistry.cpp)

######################################################
add_library(ummap-core OBJECT ${CORE_SRC})
SET_TARGET_PROPERTIES(ummap-core PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
install(FILES Driver.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ummap)

######################################################
if (ENABLE_TESTS)
	add_subdirectory(tests)
endif (ENABLE_TESTS)
