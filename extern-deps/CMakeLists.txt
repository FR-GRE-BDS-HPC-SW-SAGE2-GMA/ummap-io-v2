######################################################
#            PROJECT  : ummap-io-v2                  #
#            LICENSE  : Apache 2.0                   #
#            COPYRIGHT: 2020 Bull SAS                #
######################################################

######################################################
# add google test and disable its installation
# CAUTION on cmake 2.8 also need to override the option() line in googletest-release-1.10.0/CMakeLists.txt
if (ENABLE_TESTS)
	option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)
	add_subdirectory(googletest-release-1.10.0)
endif (ENABLE_TESTS)

######################################################
add_subdirectory(from-cern-lhcb-daqpipe-v2)

######################################################
if(MERO_FOUND)
	add_subdirectory(from-clovis-sample-apps)
endif(MERO_FOUND)
if(MOTR_FOUND)
	add_subdirectory(from-motr-sample-apps)
endif(MOTR_FOUND)
