#ifndef CONFIG_VALIDATION_HPP_
#define CONFIG_VALIDATION_HPP_

#include <string>

#include "Config.hpp"
#include "HttpRequest.hpp"
#define ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT "0123456789"
#define TCHAR "!#$%&'*+-.^_`|~" + DIGIT

bool validateHost(std::string &host);
bool validatePort(std::string &port);
bool isAllDigit(std::string &str);
bool isStatusCode(std::string &status);
bool isPath(const std::string &path);
bool isToken(std::string &str);
bool isVchar(std::string &str);
bool isFieldContent(std::string &str);
bool isMethod(const std::string &method);
bool isCGIExtension(const std::string &ext);
bool isURL(const std::string &URL);
bool is3xxStatus(std::string &status);
bool isServernameDuplicate(Config &conf);
bool isAcceptableMethod(const LocationConf *conf, const HttpRequest::Method &method);
std::string methodToString(const HttpRequest::Method &method);

template <typename Container, typename T>
bool contain(const Container &container, const T &value) {
  return std::find(container.begin(), container.end(), value) != container.end();
}

#endif
