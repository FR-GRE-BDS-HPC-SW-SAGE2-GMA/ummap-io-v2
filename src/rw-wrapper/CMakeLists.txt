######################################################
#            PROJECT  : ummap-io-v2                  #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 03/2020                      #
#            LICENSE  : ????????                     #
######################################################

######################################################
set(RW_WRAPPER_API_SRC rw-wrapper.cpp)

######################################################
add_library(ummap-io-rw-wrapper SHARED ${RW_WRAPPER_API_SRC})

######################################################
target_link_libraries(ummap-io-rw-wrapper ummap-io dl)

######################################################
install(TARGETS ummap-io-rw-wrapper DESTINATION ${CMAKE_INSTALL_LIBDIR})
