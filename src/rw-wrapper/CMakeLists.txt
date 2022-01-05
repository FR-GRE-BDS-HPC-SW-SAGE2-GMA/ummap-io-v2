######################################################
#            PROJECT  : ummap-io-v2                  #
#            VERSION  : 0.0.0-dev                    #
#            DATE     : 03/2020                      #
#            LICENSE  : ????????                     #
######################################################

######################################################
set(RW_WRAPPER_API_SRC rw-wrapper.cpp)

######################################################
add_library(ummap-io-rw-wrapper-code OBJECT ${RW_WRAPPER_API_SRC})
SET_TARGET_PROPERTIES(ummap-io-rw-wrapper-code PROPERTIES POSITION_INDEPENDENT_CODE ON)
