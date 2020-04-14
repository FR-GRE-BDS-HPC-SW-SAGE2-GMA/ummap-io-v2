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
include(GNUInstallDirs)
find_package(OpenMP REQUIRED)
find_package(Mero QUIET)


if(MERO_FOUND)
	# Compiler/Linker options for MERO example
	# Note: clovis API files do not compile with -std=c99 option
	set(MERO_COMPILE_FLAGS "-D_REENTRANT -D_GNU_SOURCE -DM0_INTERNAL='' -DM0_EXTERN=extern -fno-common -I${MERO_INCLUDE_DIRS} -DMERO_FOUND")
	set(MERO_COMPILE_FLAGS "${MERO_COMPILE_FLAGS}  -Wall -Wno-attributes -fno-strict-aliasing -rdynamic")
	set(MERO_LINK_FLAGS "-lpthread -lrt -lm -lgf_complete -lyaml -luuid -lmero")
endif(MERO_FOUND)

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
add_subdirectory(bench)


if (MERO_FOUND)
	message("MERO installation has been found, the Clovis driver and associated tests are gonna be compiled")
else (MERO_FOUND)
	message("MERO installation not found, Clovis driver is not going to be compiled")
endif(MERO_FOUND)