/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_CLOVIS_DRIVER_HPP
#define UMMAP_CLOVIS_DRIVER_HPP

/********************  HEADERS  *********************/
//internal
#include "../core/Driver.hpp"

extern "C" {
	#include "clovis_api.h"
}

/* This value is fixed based on the HW that you run, 
 * 50 for VM execution and 25 for Juelich prototype execution has been tested OK
 * Just a bit of explanation here: 
 * Internally, MERO is splitting the write ops into data units 
 * There is an arbitrary limitation within on the number of data units
 * that could be written in a single MERO ops. 
 * 
 * An error on the IO size limitation overhead looks like this: 
 * mero[XXXX]:  eb80  ERROR  [clovis/io_req.c:452:ioreq_iosm_handle_executed]  iro_dgmode_write() failed, rc=-7
 * 
 * A data unit size is directly based on the layout id of the object
 * Default layout id (defined in clovis config) for this example is 9 which leads to a data units size of 1MB
 * A value of 50 means that we can write up to 50MB (50 data units of 1MB) per ops
*/
#ifndef CLOVIS_MAX_DATA_UNIT_PER_OPS
	#define CLOVIS_MAX_DATA_UNIT_PER_OPS  50
#endif

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * This driver implement the support for the clovis API from the mero object 
 * storage.
**/
class ClovisDriver : public Driver
{
	public:
		ClovisDriver(struct m0_uint128 object_id, bool create = false);
		ClovisDriver(int64_t high, int64_t low, bool create = false);
		virtual ~ClovisDriver(void) override;
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void * ptr, size_t offset, size_t size) override;
		void setObjectId(int64_t high, int64_t low);
	private:
		/** Keep track of the object ID to be memory mapped.**/
		struct m0_uint128 m_object_id;
};

}

/**
 * Implement support of stream operator on object ID.
**/
inline std::ostream&
operator<<(std::ostream& out, struct m0_uint128 object_id)
{
  out << object_id.u_hi << ":" << object_id.u_lo;
  out << std::flush;
  return out;
}


#endif //UMMAP_CLOVIS_DRIVER_HPP
