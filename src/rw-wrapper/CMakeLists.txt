######################################################
#            PROJECT  : ummap-io-v2                  #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 03/2020                      #
#            LICENSE  : ????????                     #
######################################################

######################################################
set(RW_WRAPPER_API_SRC rw-wrapper.cpp)

######################################################
add_library(ummap-rw-wrapper OBJECT ${RW_WRAPPER_API_SRC})
SET_TARGET_PROPERTIES(ummap-rw-wrapper PROPERTIES POSITION_INDEPENDENT_CODE ON)

######################################################
target_link_libraries(ummap-rw-wrapper ummap-io dl)

######################################################
install(TARGETS ummap-rw-wrapper DESTINATION ${CMAKE_INSTALL_LIBDIR})
