/*****************************************************
			 PROJECT  : ummap-io-v2
			 LICENSE  : Apache 2.0
			 COPYRIGHT: 2020 Bull SAS
*****************************************************/

/********************  HEADERS  *********************/
#include <cstdlib>
#include "../common/Debug.hpp"
#include "HumanUnits.hpp"

/***************** USING NAMESPACE ******************/
using namespace ummapio;

/********************  CONSTS  **********************/
static const char * cstHumanMemSizeRegex = "([0-9]+)([KMGT]?B)?";

/*******************  FUNCTION  *********************/
size_t ummapio::fromHumanMemSize(const std::string & value)
{
	//var
	size_t res = 0;

	//build regex
	std::regex reg(cstHumanMemSizeRegex);
	std::smatch matches;

	//apply regexp
	if(std::regex_match(value, matches, reg)) {
		//extract
		const std::string number = matches[1];
		const std::string unit = matches[2];

		//apply
		res = atol(number.c_str());
		if (unit == "KB")
			res *= 1024ul;
		else if (unit == "MB")
			res *= 1024ul*1024ul;
		else if (unit == "GB")
			res *= 1024ul*1024ul*1024ul;
		else if (unit == "TB")
			res *= 1024ul*1024ul*1024ul*1024ul;
	} else {
		UMMAP_FATAL_ARG("Fail to match human memory size, invalid format : '%1'").arg(value).end();
	}

	//ret
	return res;
}