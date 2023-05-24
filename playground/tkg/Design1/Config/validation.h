#ifndef CONFIG_VALIDATION_HPP_
#define CONFIG_VALIDATION_HPP_

#include <string>

bool validateHost(std::string &host);
bool validatePort(std::string &port);
bool isAllDigit(const std::string &str);
bool isStatusCode(const std::string &status);
bool isPath(const std::string &path);

#endif