#ifndef CONFIG_VALIDATION_HPP_
#define CONFIG_VALIDATION_HPP_

#include <string>

#define ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT "0123456789"
#define TCHAR "!#$%&'*+-.^_`|~" +  DIGIT

bool validateHost(std::string &host);
bool validatePort(std::string &port);
bool isAllDigit(const std::string &str);
bool isStatusCode(const std::string &status);
bool isPath(const std::string &path);
bool isToken(const std::string &str);
bool isVchar(const std::string &str);

#endif
