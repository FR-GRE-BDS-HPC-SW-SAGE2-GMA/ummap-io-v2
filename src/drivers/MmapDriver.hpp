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
class MmapDriver : public Driver
{
	public:
		MmapDriver(int fd = 0, bool allowNotAligned = false);
		virtual ~MmapDriver(void) override;
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void* ptr, size_t offset, size_t size) override;
		virtual void * directMmap(size_t size, size_t offset, bool read, bool write) override;
		virtual bool directMunmap(void * base, size_t size, size_t offset) override;
		virtual bool directMSync(void * base, size_t size, size_t offset) override;
	private:
		void checkAndSetAlign(size_t & size, size_t & offset, size_t &addrOffset);
	private:
		int fd;
		bool allowNotAligned;
};

}

#endif //UMMAP_MMAP_DRIVER_HPP
