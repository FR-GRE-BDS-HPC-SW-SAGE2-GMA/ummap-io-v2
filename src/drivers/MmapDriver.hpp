/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_MMAP_DRIVER_HPP
#define UMMAP_MMAP_DRIVER_HPP

/********************  HEADERS  *********************/
//internal
#include "../core/Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*********************  CLASS  **********************/
/**
 * Provide a special driver using ummap to directly mmap a file bypassing
 * all the internals of ummap. This can be usefull on NVDIMM memory being able
 * to exploit it directly without changing the API in the application.
 * 
 * The way to proceed is mostly by providing the directMmap, directMunmap and
 * directMunmap functions from the driver creating the bypass.
**/
class MmapDriver : public Driver
{
	public:
		MmapDriver(int fd = 0, bool allowNotAligned = false);
		virtual ~MmapDriver(void) override;
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void* ptr, size_t offset, size_t size) override;
		virtual void * directMmap(void * addr, size_t size, size_t offset, bool read, bool write, bool exec, bool mapFixed) override;
		virtual bool directMunmap(void * base, size_t size, size_t offset) override;
		virtual bool directMSync(void * base, size_t size, size_t offset) override;
		void setFd(int newFd);
	private:
		void checkAndSetAlign(size_t & size, size_t & offset, size_t &addrOffset);
	private:
		/** File descriptor to be directly mapped. **/
		int fd;
		/** If we allow not aligned mappings. **/
		bool allowNotAligned;
};

}

#endif //UMMAP_MMAP_DRIVER_HPP
