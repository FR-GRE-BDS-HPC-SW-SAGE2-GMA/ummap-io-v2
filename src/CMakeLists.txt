######################################################
#  PROJECT  : ummap-io-v2                            #
#  LICENSE  : Apache 2.0                             #
#  COPYRIGHT: 2020-2021 Bull SAS All rights reserved #
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
set(OPTIONAL_EXTRA)
set(UMMAP_EXTERN_LIBS)
if (HTOPML_FOUND)
	set(OPTIONAL_EXTRA $<TARGET_OBJECTS:ummap-htopml>)
	set(HAVE_HTOPML yes)
	include_directories(${HTOPML_INCLUDE_DIRS})
	list(APPEND UMMAP_EXTERN_LIBS ${HTOPML_LIBRARY})
endif (HTOPML_FOUND)
if (IOCCLIENT_FOUND)
	set(HAVE_IOC_CLIENT yes)
endif (IOCCLIENT_FOUND)

######################################################
#setup config file
configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

######################################################
include_directories(${CMAKE_SOURCE_DIR}/extern-deps)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

######################################################
set(UMMAP_INTERNAL_CODE
	#extern deps
	$<TARGET_OBJECTS:daqpipe-common>
	#internal
	$<TARGET_OBJECTS:ummap-common>
	$<TARGET_OBJECTS:ummap-core>
	$<TARGET_OBJECTS:ummap-drivers>
	$<TARGET_OBJECTS:ummap-policies>
	$<TARGET_OBJECTS:ummap-public-api>
	$<TARGET_OBJECTS:ummap-uri>
	#we should keep an eye on portability of this
	$<TARGET_OBJECTS:ummap-portability>
	#optional
	${OPTIONAL_EXTRA}
)

######################################################
if (IOCCLIENT_FOUND)
	include_directories(${IOCCLIENT_INCLUDE_DIR})
endif (IOCCLIENT_FOUND)

######################################################
if(MERO_FOUND)
	set(UMMAP_INTERNAL_CODE ${UMMAP_INTERNAL_CODE} $<TARGET_OBJECTS:ummap-extern-mero>)
endif(MERO_FOUND)
if(MOTR_FOUND)
	set(UMMAP_INTERNAL_CODE ${UMMAP_INTERNAL_CODE} $<TARGET_OBJECTS:ummap-extern-motr>)
endif(MOTR_FOUND)

######################################################
add_library(ummap-io SHARED ${UMMAP_INTERNAL_CODE})
if(MERO_FOUND)
	set_target_properties(ummap-io PROPERTIES LINK_FLAGS "${MERO_LINK_FLAGS}")
endif(MERO_FOUND)
if(MOTR_FOUND)
	set_target_properties(ummap-io PROPERTIES LINK_FLAGS "${MOTR_LINK_FLAGS}")
endif(MOTR_FOUND)
if(IOCCLIENT_FOUND)
	target_link_libraries(ummap-io ${IOCCLIENT_LIBRARY})
endif()
target_link_libraries(ummap-io rt)
set_target_properties(ummap-io PROPERTIES VERSION 0.0.0 SOVERSION 0)
target_link_libraries(ummap-io ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(ummap-io ${UMMAP_EXTERN_LIBS})
install(TARGETS ummap-io DESTINATION ${CMAKE_INSTALL_LIBDIR})

######################################################
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(drivers)
add_subdirectory(portability)
add_subdirectory(policies)
add_subdirectory(uri)
add_subdirectory(public-api)

######################################################
if (HTOPML_FOUND)
	add_subdirectory(htopml)
endif (HTOPML_FOUND)
