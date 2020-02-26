######################################################
#            PROJECT  : ummap-io-v2                  #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 02/2020                      #
#            LICENSE  : CeCILL-C                     #
######################################################

######################################################
# add google test and disable its installation
# CAUTION on cmake 2.8 also need to override the option() line in googletest-release-1.10.0/CMakeLists.txt
option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)
add_subdirectory(googletest-release-1.10.0)
