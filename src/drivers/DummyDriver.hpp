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
namespace ummapio
{

/*********************  CLASS  **********************/
class DummyDriver : public Driver
{
	public:
		DummyDriver(char value = 0);
		virtual ~DummyDriver(void) override;
		virtual ssize_t pwrite(const void * buffer, size_t size, size_t offset) override;
		virtual ssize_t pread(void * buffer, size_t size, size_t offset) override;
		virtual void sync(void * ptr, size_t offset, size_t size) override;
	private:
		char value;
};

}

#endif //UMMAP_DUMMY_DRIVER_HPP
