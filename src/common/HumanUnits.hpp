/*****************************************************
*  PROJECT  : ummap-io-v2                            *
*  LICENSE  : Apache 2.0                             *
*  COPYRIGHT: 2020-2021 Bull SAS All rights reserved *
*****************************************************/

#ifndef UMMAP_HUMAN_UNITS_HPP
#define UMMAP_HUMAN_UNITS_HPP

/********************  HEADERS  *********************/
#include <string>
#include <regex>

/********************  NAMESPACE  *******************/
namespace ummapio
{

/*******************  FUNCTION  *********************/
size_t fromHumanMemSize(const std::string & value);
size_t fromHumanMemSizeRegexp(const std::string & value);
size_t fromHumanMemSizeNoRegexp(const std::string & value);

}

#endif //UMMAP_HUMAN_UNITS_HPP
