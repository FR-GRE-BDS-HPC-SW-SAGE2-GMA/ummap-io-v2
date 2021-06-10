######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
# Basic
cmake_minimum_required(VERSION 2.8)
project(ummap-io-v2 CXX C)

######################################################
# Tests
set(ENABLE_TESTS ON CACHE BOOL "Enable unit tests requiring build of googletest and googlmock")
enable_testing()

######################################################
#Add internal search path for scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)

######################################################
# import
include(${CMAKE_SOURCE_DIR}/cmake/macros.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/unittest-macros.cmake)

######################################################
# dependencies
include(GNUInstallDirs)
find_package(OpenMP REQUIRED)
find_package(Threads REQUIRED)
find_package(Htopml QUIET)
find_package(Mero QUIET)
find_package(Motr QUIET)
find_package(IocClient QUIET)

######################################################
# mero
if(MERO_FOUND)
	# Compiler/Linker options for MERO example
	# Note: clovis API files do not compile with -std=c99 option
	set(MERO_COMPILE_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -fno-common -I${MERO_INCLUDE_DIRS}")
	set(MERO_COMPILE_FLAGS "${MERO_COMPILE_FLAGS}  -Wall -Wno-attributes -fno-strict-aliasing -rdynamic")
	set(MERO_LINK_FLAGS "-lpthread -lrt -lm -lgf_complete -lyaml -luuid -lmero")
	set(HAVE_MERO yes)
	include_directories(${CMAKE_SOURCE_DIR}/extern-deps/from-clovis-sample-apps/)
endif(MERO_FOUND)
if(MOTR_FOUND)
	# Compiler/Linker options for MERO example
	# Note: clovis API files do not compile with -std=c99 option
	set(MOTR_COMPILE_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -fno-common -I${MOTR_INCLUDE_DIRS}")
	set(MOTR_COMPILE_FLAGS "${MOTR_COMPILE_FLAGS}  -Wall -Wno-attributes -fno-strict-aliasing -rdynamic")
	set(MOTR_LINK_FLAGS "-lpthread -lrt -lm -lyaml -luuid -lmotr")
	set(HAVE_MOTR yes)
	include_directories(${CMAKE_SOURCE_DIR}/extern-deps/from-motr-sample-apps/)
endif(MOTR_FOUND)

######################################################
# setup
ummap_setup_internal_gmock_and_gtest()
ummap_enable_cxx_11()
if (ENABLE_COVERAGE)
	ummap_enable_gcc_coverage()
endif(ENABLE_COVERAGE)

######################################################
add_subdirectory(extern-deps)
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(bench)

######################################################
if (MERO_FOUND)
	message("MERO installation has been found, the Clovis driver and associated tests are gonna be compiled")
endif(MERO_FOUND)
if (MERO_FOUND)
	message("MOTR installation has been found, the Motr driver and associated tests are gonna be compiled")
endif(MERO_FOUND)
if (NOT MERO_FOUND AND NOT MOTR_FOUND)
	message("MERO/MOTR installation not found, Clovis/Motr driver is not going to be compiled")
endif(NOT MERO_FOUND AND NOT MOTR_FOUND)
