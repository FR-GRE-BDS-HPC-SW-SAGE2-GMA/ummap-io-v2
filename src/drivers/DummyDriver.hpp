/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

#ifndef UMMAP_DUMMY_DRIVER_HPP
#define UMMAP_DUMMY_DRIVER_HPP

/********************  HEADERS  *********************/
//internal
#include "../core/Driver.hpp"

/********************  NAMESPACE  *******************/
namespace ummap
{

/*********************  CLASS  **********************/
class DummyDriver : public Driver
{
	public:
		DummyDriver(char value = 0);
		virtual ~DummyDriver(void);
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset);
		virtual ssize_t pread(void * buffer, size_t size, size_t offset);
		virtual void sync(size_t offset, size_t size);
		virtual Driver * dup(void);
	private:
		char value;
};

}

#endif //UMMAP_DUMMY_DRIVER_HPP
