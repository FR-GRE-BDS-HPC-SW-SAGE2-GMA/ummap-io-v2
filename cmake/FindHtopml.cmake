######################################################
# - Try to find htopml (https://github.com/svalat/htopml)
# Once done this will define
#  HTOPML_FOUND - System has htopml
#  HTOPML_INCLUDE_DIRS - The htopml include directories
#  HTOPML_LIBRARIES - The libraries needed to use htopml
#  HTOPML_DEFINITIONS - Compiler switches required for using htopml

######################################################
set(HTOPML_PREFIX "" CACHE STRING "Help cmake to find htopml library (https://github.com/svalat/htopml) into your system.")

######################################################
find_path(HTOPML_INCLUDE_DIR htopml/Htopml.h
	HINTS ${HTOPML_PREFIX}/include)

######################################################
find_library(HTOPML_LIBRARY NAMES htopml
	HINTS ${HTOPML_PREFIX}/lib)

######################################################
set(HTOPML_LIBRARIES ${HTOPML_LIBRARY} )
set(HTOPML_INCLUDE_DIRS ${HTOPML_INCLUDE_DIR} )

######################################################
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set HTOPML_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Htopml  DEFAULT_MSG
	HTOPML_LIBRARY HTOPML_INCLUDE_DIR)

######################################################
mark_as_advanced(HTOPML_INCLUDE_DIR HTOPML_LIBRARY HTOPML_LIBRARIES HTOPML_INCLUDE_DIRS)
