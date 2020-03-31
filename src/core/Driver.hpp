/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_DRIVER_HPP
#define UMMAP_DRIVER_HPP

/********************  HEADERS  *********************/
//std
#include <cstdlib>

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
/**
 * Base class to implement the IO driver to be pluggin inside
 * the Mapping. It mostly provide read/write/sync/dup operations.
**/
class Driver
{
	public:
		Driver(void);
		virtual ~Driver(void);
		/**
		 * Apply write operation to dump the memory content into the related IO storage.
		 * @param buffer Buffer pointer containing the data to write.
		 * @param size Size of the data to write.
		 * @param offset Offset in the storage element.
		 * @return The writted size.
		**/
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) = 0;
		/**
		 * Apply read operation to load the content into memory from the related IO storage.
		 * @param buffer Buffer pointer in which to load the data.
		 * @param size Size of the data to read.
		 * @param offset Offset in the storage element.
		 * @return The read size.
		**/
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) = 0;
		/**
		 * Apply a sync operation on the given interval.
		 * @param offset From where to start the sync operation.
		 * @param size Size of the segment to sync.
		**/
		virtual void sync(size_t offset, size_t size) = 0;
		void setAutoclean(bool status = true);
		bool hasAutoclean(void) const;
	private:
		bool autoclean;
};

}

#endif //UMMAP_DRIVER_HPP
