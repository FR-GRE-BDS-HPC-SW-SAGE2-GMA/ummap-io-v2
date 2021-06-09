/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
//std
#include <cstring>
#include <cassert>
#include <iostream>
//internal
#include "ClovisDriver.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;
using namespace std;


/*******************  FUNCTION  *********************/
ClovisDriver::ClovisDriver(struct m0_uint128 object_id, bool create)
{

	m_object_id.u_lo = object_id.u_lo;
	m_object_id.u_hi = object_id.u_hi;
	if (create)
		create_object(m_object_id);
}

/*******************  FUNCTION  *********************/
ClovisDriver::ClovisDriver(int64_t high, int64_t low, bool create)
{
	m_object_id.u_lo = low;
	m_object_id.u_hi = high;
	if (create)
		create_object(m_object_id);
}

/*******************  FUNCTION  *********************/
ClovisDriver::~ClovisDriver(void)
{
}

/****************************************************/
ssize_t StorageBackendMero::pread(int64_t high, int64_t low, void * buffer, size_t size, size_t offset)
{
	//check
	assert(buffer != NULL);

	#if defined(HAVE_MOTR)
		assume(size % 4096 == 0, "Clovis driver work only with size multiple of the page size !");
		assume(offset % 4096 == 0, "Clovis driver work only with offset multiple of the page size !");

		//check mero block size
		int pool = 0;
		size_t m0bs = c0appz_m0bs(high, low, size, pool);

		//make read
		size_t bsz = 4096;
		size_t cnt = size / bsz;
		int ret = c0appz_mr((char*)buffer, high, low, offset, bsz, cnt, m0bs);

		//set object for debug
		struct m0_uint128 m_object_id;
		m_object_id.u_hi = high;
		m_object_id.u_lo = low;

		//check status
		if (ret == 0) {
			IOC_DEBUG_ARG("mero", "Success executing the MERO helperPread op, object ID=%1, offset=%3, size=%2")
				.arg(m_object_id)
				.arg(offset)
				.arg(size)
				.end();
			return size;
		} else {
			cerr << "[Failed] Error executing the MERO helperPread op, object ID=" << m_object_id << " , size=" << size
						<< ", offset=" << offset << endl;
			errno = EIO;
			return -1;
		}
	#elif defined(HAVE_MERO)
		struct m0_indexvec ext;
		struct m0_bufvec data;
		struct m0_bufvec attr;

		struct m0_uint128 m_object_id;
		m_object_id.u_hi = high;
		m_object_id.u_lo = low;

		char *char_buf = (char *)buffer;
		int ret = 0;
		// We assume here 2 things: 
		// -> That the object is opened
		// -> And opened with the same layout as the default clovis one
		int layout_id = m0_clovis_layout_id(clovis_instance);
		size_t data_units_size = (size_t) m0_clovis_obj_layout_id_to_unit_size(layout_id);

		assert(data_units_size > 0);

		int total_blocks_to_read = 0;

		if (data_units_size > size) {
			data_units_size = size;
			total_blocks_to_read = 1;        
		} else {
			total_blocks_to_read =  size / data_units_size;
			if (size % data_units_size != 0) {
				assert(false && "We don't handle the case where the IO size is not a multiple of the data units");
			}
		}

		int last_index = offset;
		int j = 0;

		while(total_blocks_to_read > 0) {

			int block_size = (total_blocks_to_read > CLOVIS_MAX_DATA_UNIT_PER_OPS)?
						CLOVIS_MAX_DATA_UNIT_PER_OPS : total_blocks_to_read;

			m0_bufvec_alloc(&data, block_size, data_units_size);
			m0_bufvec_alloc(&attr, block_size, 1);
			m0_indexvec_alloc(&ext, block_size);

				/* Initialize the different arrays */
			int i;
			for (i = 0; i < block_size; i++) {
				attr.ov_vec.v_count[i] = 1;
				ext.iv_index[i] = last_index;
				ext.iv_vec.v_count[i] = data_units_size;
				last_index += data_units_size;
			}

			ret = read_data_from_object(m_object_id, &ext, &data, &attr);
			// @todo: handling partial read 
			if (ret != 0)
				break;

			for (i = 0; i < block_size; i++) {
				memcpy((char_buf + i*data_units_size + j*block_size*data_units_size), data.ov_buf[i], data_units_size);
			}

			m0_indexvec_free(&ext);
			m0_bufvec_free(&data);
			m0_bufvec_free(&attr);
			
			total_blocks_to_read -= block_size;
			j++;
		};

		if (ret == 0) {
			IOC_DEBUG_ARG("mero", "Success executing the MERO helperPread op, object ID=%1, offset=%3, size=%2")
				.arg(m_object_id)
				.arg(offset)
				.arg(size)
				.end();
			return size;
		} else {
			cerr << "[Failed] Error executing the MERO helperPread op, object ID=" << m_object_id << " , size=" << size
						<< ", offset=" << offset << endl;
			errno = EIO;
			return -1;
		}
	#else
		return -1;
	#endif
}

/****************************************************/
ssize_t ClovisDriver::pwrite(int64_t high, int64_t low, void * buffer, size_t size, size_t offset)
{
	//check
	assert(buffer != NULL);

	#if defined(HAVE_MOTR)
		assume(size % 4096 == 0, "Motr driver work only with size multiple of the page size !");
		assume(offset % 4096 == 0, "Motr driver work only with offset multiple of the page size !");

		//check mero block size
		int pool = 0;
		size_t m0bs = c0appz_m0bs(high, low, size, pool);

		//make read
		size_t bsz = 4096;
		size_t cnt = size / bsz;
		int ret = c0appz_mw((char*)buffer, high, low, offset, bsz, cnt, m0bs);

		//set object for debug
		struct m0_uint128 m_object_id;
		m_object_id.u_hi = high;
		m_object_id.u_lo = low;

		//check status
		if (ret == 0) {
			IOC_DEBUG_ARG("mero", "Success executing the MERO helperWrite op, object ID=%1, offset=%3, size=%2")
				.arg(m_object_id)
				.arg(offset)
				.arg(size)
				.end();
			return size;
		} else {
			cerr << "[Failed] Error executing the MERO helperWrite op, object ID=" << m_object_id << " , size=" << size
						<< ", offset=" << offset << endl;
			errno = EIO;
			return -1;
		}
	#elif defined(HAVE_MERO)
		struct m0_indexvec ext;
		struct m0_bufvec data;
		struct m0_bufvec attr;

		char *char_buf = (char *) buffer;
		int ret = 0;

		struct m0_uint128 m_object_id;
		m_object_id.u_hi = high;
		m_object_id.u_lo = low;

		// We assume here 2 things:
		// -> That the object is opened
		// -> And opened with the same layout as the default clovis one
		int layout_id = m0_clovis_layout_id(clovis_instance);
		size_t data_units_size = (size_t) m0_clovis_obj_layout_id_to_unit_size(layout_id);

		assert(data_units_size > 0);

		int total_blocks_to_write = 0;

		if (data_units_size > size) {
			data_units_size = size;
			total_blocks_to_write = 1;        
		} else {
			total_blocks_to_write =  size / data_units_size;
			if (size % data_units_size != 0) {
				assert(false && "We don't handle the case where the IO size is not a multiple of the data units");
			}
		}

		int last_index = offset;
		int j = 0;

		while(total_blocks_to_write > 0) {

			int block_size = (total_blocks_to_write > CLOVIS_MAX_DATA_UNIT_PER_OPS)?
						CLOVIS_MAX_DATA_UNIT_PER_OPS : total_blocks_to_write;

			m0_bufvec_alloc(&data, block_size, data_units_size);
			m0_bufvec_alloc(&attr, block_size, 1);
			m0_indexvec_alloc(&ext, block_size);

				/* Initialize the different arrays */
			int i;
			for (i = 0; i < block_size; i++) {

				//@todo: Can we remove this extra copy ?
				memcpy(data.ov_buf[i], (char_buf + i*data_units_size + j*block_size*data_units_size), data_units_size);

				attr.ov_vec.v_count[i] = 1;

				ext.iv_index[i] = last_index;
				ext.iv_vec.v_count[i] = data_units_size;
				last_index += data_units_size;
			}

			cout << "[Pending] Send MERO write ops, object ID=" << m_object_id;
			cout << ", size=" << block_size << ", bs=" << data_units_size << endl;
			
			// Send the write ops to MERO and wait for completion 
			ret = write_data_to_object(m_object_id, &ext, &data, &attr);

			m0_indexvec_free(&ext);
			m0_bufvec_free(&data);
			m0_bufvec_free(&attr);

			// @todo: handling partial write
			if (ret != 0) 
				break;
			
			total_blocks_to_write -= block_size;
			j++;
		};

		if (ret == 0) {
			IOC_DEBUG_ARG("mero", "Success executing the MERO helperWrite op, object ID=%1, offset=%3, size=%2")
				.arg(m_object_id)
				.arg(offset)
				.arg(size)
				.end();
			return size;
		} else {
			cerr << "[Failed] Error executing the MERO helperPwrite op, object ID=" << m_object_id << " , size=" << size
						<< ", offset=" << offset << endl;
			errno = EIO;
			return -1;
		}
	#else
		return -1;
	#endif
}

/*******************  FUNCTION  *********************/
void ClovisDriver::sync(void * ptr, size_t offset, size_t size)
{
	//cerr << "[Failed] Sync op for the Clovis driver is not implemented" << endl;
	// Not a problem as the clovis are already synchronous 
}

/*******************  FUNCTION  *********************/
void ClovisDriver::setObjectId(int64_t high, int64_t low)
{
	this->m_object_id.u_hi = high;
	this->m_object_id.u_lo = low;
}
