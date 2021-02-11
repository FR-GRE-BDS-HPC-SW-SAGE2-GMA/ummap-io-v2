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
#include <string>

/********************  NAMESPACE  *******************/
namespace ummapio
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
		 * @param ptr The pointer of the segment to sync (without accounting offset which need to be added).
		 * @param offset From where to start the sync operation.
		 * @param size Size of the segment to sync.
		**/
		virtual void sync(void *ptr, size_t offset, size_t size) = 0;
		/**
		 * Let the driver making the memory mapping. This is to be used
		 * by direct access modes
		 * @param addr Define the optional address where to map.
		 * @param size Define the mapping size.
		 * @param offset Define the offset in file.
		 * @param read Accept read accesses
		 * @param write Accept write accesses
		 * @param mapFixed Force the mapping address to addr.
		 * @return NULL if the caller has to do the mapping, an addresse otherwise.
		**/
		virtual void * directMmap(void * addr, size_t size, size_t offset, bool read, bool write, bool exec, bool mapFixed);
		/**
		 * Let the driver maling the memory unmaping.
		 * @param base Base addresse of the mapping.
		 * @param size Size of the mapping.
		 * @param offset Define the offset in file.
		 * @return Return true if it has unmapped, false if it need to be done by the caller.
		**/
		virtual bool directMunmap(void * base, size_t size, size_t offset);
		/**
		 * When directly mapping do not make the ummap internal state tracking but direct msync
		 * @param base Base addresse of the mapping.
		 * @param size Size of the mapping.
		 * @param offset Define the offset in file.
		**/
		virtual bool directMSync(void * base, size_t size, size_t offset);
		/**
		 * Notify the driver that we stablish a new mapping 
		 * @param offset Offset of the mapping in the storage.
		 * @param size Size of the mapping.
		 * @param write If the mapping is in write mode.
		**/
		virtual int64_t establish_mapping(size_t offset, size_t size, bool write);
		/**
		 * Notify the driver that erase a mapping 
		 * @param offset Offset of the mapping in the storage.
		 * @param size Size of the mapping.
		 * @param write If the mapping is in write mode.
		**/
		virtual void erase_mapping(int64_t data, size_t offset, size_t size, bool write);
		void setAutoclean(bool status = true);
		bool hasAutoclean(void) const;
		void setUri(const std::string & uri);
		const std::string & getUri(void) const;
	private:
		bool autoclean;
		std::string uri;
};

}

#endif //UMMAP_DRIVER_HPP
