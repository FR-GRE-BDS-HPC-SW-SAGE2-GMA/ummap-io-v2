/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/** This file provide the glue to call daqpipe debug functions **/

#ifndef UMMAP_DEBUG_HPP
#define UMMAP_DEBUG_HPP

/********************  HEADERS  *********************/
#include <from-cern-lhcb-daqpipe-v2/Debug.hpp>

/********************  NAMESPACE  *******************/
namespace ummap_io
{

/********************  MACROS  **********************/
#define UMMAP_FATAL(x)   DAQ_FATAL(x)
#define UMMAP_DEBUG(cat,x) DAQ_DEBUG(cat,x)
#define UMMAP_ERROR(x)     DAQ_ERROR(x)
#define UMMAP_WARNING(x)   DAQ_WARNING(x)
#define UMMAP_MESSAGE(x)   DAQ_MESSAGE(x)
#define UMMAP_INFO(x)      DAQ_INFO(x)

/********************  MACROS  **********************/
#define UMMAP_FATAL_ARG(x)     DAQ_FATAL_ARG(x)
#define UMMAP_ERROR_ARG(x)     DAQ_ERROR_ARG(x)
#define UMMAP_WARNING_ARG(x)   DAQ_WARNING_ARG(x)
#define UMMAP_MESSAGE_ARG(x)   DAQ_MESSAGE_ARG(x)
#define UMMAP_INFO_ARG(x)      DAQ_INFO_ARG(x)
#define UMMAP_DEBUG_ARG(cat,x) DAQ_DEBUG_ARG(cat,x)

/********************  MACROS  **********************/
#define assume(check,message) do { if (!(check)) DAQ_FATAL(message); } while(0)
#define assumeArg(check,message) if (!(check)) DAQ_FATAL_ARG(message)

/********************  MACROS  **********************/
#define DAQ_TO_STRING(x) #x
#ifdef NDEBUG
	#define UMMAP_ASSERT(x)      do{} while(0)
#else
	#define UMMAP_ASSERT(x)      do{ if (!(x)) DAQ::Debug(DAQ_TO_STRING(x),DAQ_CODE_LOCATION,DAQ::MESSAGE_ASSERT).end(); } while(0)
#endif

}

#endif //UMMAP_DEBUG_HPP
