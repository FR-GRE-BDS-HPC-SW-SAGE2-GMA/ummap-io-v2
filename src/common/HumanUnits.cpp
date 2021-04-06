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
/**
 * Define the regular expression to check allowed human readable size units 
 * It allow notation like 4096, 4KB, 8MB, 16GB, 32TB.
**/
static const char * cstHumanMemSizeRegex = "([0-9]+)([KMGT]?B)?";

/*******************  FUNCTION  *********************/
/**
 * Parse the human readable size without using regular expression.
 * @param value Define the size to parse as a string. Eg. 8MB.
 * @return Return the parsed size in bytes.
**/
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
	if (unit == "") {
		//nothing to do
	} else if (unit == "KB") {
		res *= 1024ul;
	} else if (unit == "MB") {
		res *= 1024ul*1024ul;
	} else if (unit == "GB") {
		res *= 1024ul*1024ul*1024ul;
	} else if (unit == "TB") {
		res *= 1024ul*1024ul*1024ul*1024ul;
	} else {
		UMMAP_FATAL_ARG("Invalid unit: %1").arg(value).end();
	}

	//ret
	return res;
}

/**
 * Parse the human readable size by using regular expression.
 * @param value Define the size to parse as a string. Eg. 8MB.
 * @return Return the parsed size in bytes.
**/
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
/**
 * Select the parsing mode depending on availability of regular
 * expression support which is not complete on old system.
 * The regular expression mode is more strict on check to detect errors.
**/
size_t ummapio::fromHumanMemSize(const std::string & value)
{
	try {
		return fromHumanMemSizeRegexp(value);
	} catch (std::exception & e) {
		return fromHumanMemSizeNoRegexp(value);
	}
}