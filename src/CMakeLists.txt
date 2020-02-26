######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
#vars
set(WITH_OS "Unix" CACHE STRING "On which OS we are running on to adapt system functions and rules. Currently only support 'Unix")

######################################################
#default
set(UMMAP_PORTABILITY_OS_UNIX OFF)
set(UMMAP_PORTABILITY_SPINLOCK_PTHREAD OFF)
set(UMMAP_PORTABILITY_SPINLOCK_DUMMY OFF)
set(UMMAP_PORTABILITY_SPINLOCK_APPLE OFF)

######################################################
#portabilty tricks
if (WITH_OS STREQUAL "Unix")
	set(UMMAP_PORTABILITY_OS_UNIX ON)
	if (CMAKE_SYSTEM_NAME STREQUAL "APPLE")
	set(UMMAP_PORTABILITY_SPINLOCK_APPLE ON)
	else()
		set(UMMAP_PORTABILITY_SPINLOCK_PTHREAD ON)
	endif()
endif()

######################################################
#setup config file
configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

######################################################
include_directories(${CMAKE_SOURCE_DIR}/extern-deps)

######################################################
set(UMMAP_INTERNAL_CODE
	#extern deps
	$<TARGET_OBJECTS:daqpipe-common>
	#internal
	$<TARGET_OBJECTS:ummap-core>
	$<TARGET_OBJECTS:ummap-drivers>
	#$<TARGET_OBJECTS:ummap-policies>
	#$<TARGET_OBJECTS:ummap-common>
	#we should keep an eye on portability of this
	$<TARGET_OBJECTS:ummap-portability>
)

######################################################
add_library(ummap-io SHARED ${UMMAP_INTERNAL_CODE})

######################################################
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(drivers)
add_subdirectory(portability)
add_subdirectory(policies)
add_subdirectory(public-api)
