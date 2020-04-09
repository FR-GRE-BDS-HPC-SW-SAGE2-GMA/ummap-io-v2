######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
# Basic
cmake_minimum_required(VERSION 2.8)
project(ummap-io-v2 CXX)

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
find_package(Threads REQUIRED)
find_package(Htopml QUIET)

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
