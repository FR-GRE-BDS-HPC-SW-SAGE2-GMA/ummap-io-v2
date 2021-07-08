######################################################
# - Try to find ioc-client
# Once done this will define
#  IOCCLIENT_FOUND - System has ioc-client
#  IOCCLIENT_INCLUDE_DIRS - The ioc-client include directories
#  IOCCLIENT_LIBRARIES - The libraries needed to use ioc-client
#  IOCCLIENT_DEFINITIONS - Compiler switches required for using ioc-client

######################################################
set(IOCCLIENT_PREFIX "" CACHE STRING "Help cmake to find ioc-client library into your system.")

######################################################
find_path(IOCCLIENT_INCLUDE_DIR ioc-client.h
	HINTS ${IOCCLIENT_PREFIX}/include)

######################################################
find_library(IOCCLIENT_LIBRARY NAMES ioc-client
	HINTS ${IOCCLIENT_PREFIX}/lib)

######################################################
set(IOCCLIENT_LIBRARIES ${IOCCLIENT_LIBRARY} )
set(IOCCLIENT_INCLUDE_DIRS ${IOCCLIENT_INCLUDE_DIR} )

######################################################
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set IOCCLIENT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(IocClient  DEFAULT_MSG
	IOCCLIENT_LIBRARY IOCCLIENT_INCLUDE_DIR)

######################################################
mark_as_advanced(IOCCLIENT_INCLUDE_DIR IOCCLIENT_LIBRARY IOCCLIENT_LIBRARIES IOCCLIENT_INCLUDE_DIRS)
