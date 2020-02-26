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
set(PORTABILITY_OS_UNIX OFF)
#portabilty twicks
if (WITH_OS STREQUAL "Unix")
	set(UMMAP_PORTABILITY_OS_UNIX ON)
endif()

######################################################
#setup config file
configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

######################################################
include_directories(${CMAKE_SOURCE_DIR}/extern-deps)

######################################################
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(drivers)
add_subdirectory(portability)
add_subdirectory(policies)
add_subdirectory(public-api)
