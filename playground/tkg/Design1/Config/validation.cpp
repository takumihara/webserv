#include "validation.h"

#include <cstdlib>
#include <iostream>

#include "../const.hpp"
#include "Config.hpp"

size_t skipSep(std::string &str, std::string sep, size_t pos) {
  while (str.find(sep, pos) == 0) {
    pos += sep.size();
  }
  return pos;
}

std::vector<std::string> split(std::string &str, const std::string &sep) {
  size_t head = 0;
  size_t end = str.find(sep);
  std::vector<std::string> ret;

  while (head < str.size()) {
    ret.push_back(str.substr(head, end - head));
    head = end + sep.size();
    end = str.find(sep, head);
    if (end == std::string::npos) {
      end = str.size();
    }
  }
  return ret;
}

bool isStatusCode(const std::string &status) { return status.size() == 3 && isAllDigit(status); }

bool isAllDigit(const std::string &str) {
  if (str == "") {
    return false;
  }
  for (std::string::const_iterator itr = str.cbegin(); itr != str.cend(); itr++) {
    if (*itr < '0' || '9' < *itr) return false;
  }
  return true;
}

bool is3xxStatus(const std::string &status) {
  if (isAllDigit(status) && status[0] == '3') return true;
  return false;
}

bool isPath(const std::string &path) {
  // todo: verify valid path
  return path.size() != 0;
}

bool isURL(const std::string &URL) {
  // todo: verify valid URL
  return URL.size() != 0;
}

bool isMethod(const std::string &method) {
  if (method == "GET" || method == "POST" || method == "DELETE" || method == "HEAD") return true;
  return false;
}

bool isCGIExtension(const std::string &ext) {
  if (ext == ".cgi" || ext == ".php" || ext == ".py") return true;
  return false;
}

bool validateHost(std::string &host) {
  if (host == "") return true;
  std::vector<std::string> split_host = split(host, ".");
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

// todo(thara): this can be more effective
// token = 1*tchar (https://triple-underscore.github.io/RFC7230-ja.html#field.components)
bool isToken(const std::string &str) {
  if (str == "") {
    return false;
  }
  for (std::string::const_iterator itr = str.cbegin(); itr != str.cend(); itr++) {
    if (tchar().find(*itr) == std::string::npos) return false;
  }
  return true;
}

bool isVchar(const std::string &str) {
  if (str == "") {
    return false;
  }
  for (std::string::const_iterator itr = str.cbegin(); itr != str.cend(); itr++) {
    if (vchar().find(*itr) == std::string::npos) return false;
  }
  return true;
}

bool isServernameDuplicate(Config &conf) {
  std::map<std::string, bool> checklist;
  for (std::vector<ServerConf>::iterator serv = conf.server_confs_.begin(); serv != conf.server_confs_.end(); serv++) {
    for (std::vector<std::string>::iterator name = serv->server_names_.begin(); name != serv->server_names_.end();
         name++) {
      if (checklist.find(*name) != checklist.end()) return false;
      checklist[*name] = true;
    }
  }
  return true;
}
