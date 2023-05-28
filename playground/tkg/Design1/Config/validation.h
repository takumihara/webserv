#ifndef CONFIG_VALIDATION_HPP_
#define CONFIG_VALIDATION_HPP_

#include <string>

#include "Config.hpp"

bool validateHost(std::string &host);
bool validatePort(std::string &port);
bool isAllDigit(const std::string &str);
bool isStatusCode(const std::string &status);
bool isPath(const std::string &path);
bool isMethod(const std::string &method);
bool isCGIExtension(const std::string &ext);
bool isURL(const std::string &URL);
bool is3xxStatus(const std::string &status);
bool isServernameDuplicate(Config &conf);

#endif