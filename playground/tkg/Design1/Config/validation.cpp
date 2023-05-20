#include "validation.h"

#include <cstdlib>
#include <iostream>

#include "Config.hpp"

size_t skipSep(std::string &str, std::string sep, size_t pos) {
  while (str.find(sep, pos) == 0) {
    pos += sep.size();
  }
  return pos;
}

std::vector<std::string> split(std::string &str, std::string &sep) {
  size_t head = 0;
  size_t end = str.find(".");
  std::vector<std::string> ret;

  while (head < str.size()) {
    ret.push_back(str.substr(head, end - head));
    head = end + sep.size();
    // head = skipSep(str, sep, head);
    end = str.find(sep, head);
    if (end == std::string::npos) {
      end = str.size();
    }
  }
  return ret;
}

bool isAllDigit(const std::string &str) {
  if (str == "") {
    return false;
  }
  for (std::string::const_iterator itr = str.cbegin(); itr != str.cend(); itr++) {
    if (*itr < '0' || '9' < *itr) return false;
  }
  return true;
}

bool validateHost(std::string &host) {
  if (host == "") return true;
  std::string sep(".");
  std::vector<std::string> split_host = split(host, sep);
  if (split_host.size() != 4) return false;
  for (std::vector<std::string>::const_iterator itr = split_host.cbegin(); itr != split_host.cend(); itr++) {
    if (!isAllDigit(*itr)) return false;
    int num = atoi((*itr).c_str());
    if (num < 0 || 255 < num) return false;
  }
  return true;
}

bool validatePort(std::string &port) {
  if (port == "") return true;
  long num_port = atol(port.c_str());
  if (0 <= num_port && num_port < UINT16_MAX) {
    return true;
  }
  return false;
}
