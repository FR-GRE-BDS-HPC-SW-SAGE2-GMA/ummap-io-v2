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
static size_t fromHumanMemSizeNoRegexp(const std::string & value)
{
	//vars
	std::string number;
	std::string unit;

	//loop
	for (size_t i = 0 ; i < value.size() ; i++){
		if (value[i] >= '0' && value[i] <= '9') {
			assumeArg(unit.empty(), "Fail to match human memory size, invalid format : '%1'").arg(value).end();
			number += value[i];
		} else if (value[i] == 'K' || value[i] == 'M' || value[i] == 'G' || value[i] == 'T' || value[i] == 'B' ) {
			unit += value[i];
		} else {
			UMMAP_FATAL_ARG("Fail to match human memory size, invalid format : '%1'").arg(value).end();
		}
	}
	//apply
	size_t res = atol(number.c_str());
	assumeArg(res > 0, "Invalid null size : %1\n").arg(value).end();
	if (unit == "KB")
		res *= 1024ul;
	else if (unit == "MB")
		res *= 1024ul*1024ul;
	else if (unit == "GB")
		res *= 1024ul*1024ul*1024ul;
	else if (unit == "TB")
		res *= 1024ul*1024ul*1024ul*1024ul;

	//ret
	return res;
}

static size_t fromHumanMemSizeRegexp(const std::string & value)
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
		assumeArg(res > 0, "Invalid null size : %1\n").arg(value).end();
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

/*******************  FUNCTION  *********************/
size_t ummapio::fromHumanMemSize(const std::string & value)
{
	try {
		return fromHumanMemSizeRegexp(value);
	} catch (std::exception & e) {
		return fromHumanMemSizeNoRegexp(value);
	}
}