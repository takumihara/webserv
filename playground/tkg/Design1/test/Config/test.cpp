#if defined(CONFTEST)
#include <gtest/gtest.h>

#include "Config.hpp"
#include "Parser.hpp"
#else
#include "../../src/Config/Config.hpp"
#include "../../src/Config/Parser.hpp"
#endif

#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

static std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("conf file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

std::string stringifyAutoindex(const CommonConf &conf, int indent) {
  std::string ret = "";
  for (int i = 0; i < indent; i++) {
    ret += "  ";
  }
  ret += "autoindex: ";
  if (conf.autoindex_)
    ret += "on\n";
  else
    ret += "off\n";
  return ret;
}

template <class T>
std::string stringifyRedirect(const T &conf, int indent) {
  std::string ret = "";
  if (conf.redirect_.first == "") return "";
  for (int i = 0; i < indent; i++) {
    ret += "  ";
  }
  ret += "redirect: " + conf.redirect_.first + " " + conf.redirect_.second + "\n";
  return ret;
}

std::string createStrings(const std::string &prefix, const std::vector<std::string> &strs) {
  std::string ret = "";
  if (strs.size() == 0) return "";
  ret += prefix;
  for (std::vector<std::string>::const_iterator itr = strs.cbegin(); itr != strs.cend(); itr++) {
    if (itr != strs.cbegin()) {
      ret += " ";
    }
    ret += *itr;
  }
  ret += "\n";
  return ret;
}

std::string stringifyErrorPages(const std::string &prefix, const std::map<std::string, std::string> &error_pages) {
  std::string ret = "";
  if (error_pages.size() == 0) return "";
  for (std::map<std::string, std::string>::const_iterator itr = error_pages.cbegin(); itr != error_pages.cend();
       itr++) {
    ret += prefix + itr->first + " " + itr->second + "\n";
  }
  return ret;
}

std::string stringifyCommon(const CommonConf &conf, int indent) {
  std::string ret = "";
  std::string spaces = "";
  for (int i = 0; i < indent; i++) {
    spaces += "  ";
  }
  char buff[100];
  snprintf(buff, sizeof(buff), "max_body_size: %zu\n", conf.max_body_size_);
  ret += spaces + buff;
  ret += spaces + "root: " + conf.root_ + "\n";
  ret += createStrings(spaces + "index: ", conf.index_);
  ret += stringifyAutoindex(conf, indent);
  ret += stringifyErrorPages(spaces + "error_page: ", conf.error_pages_);
  return ret;
}

std::string stringifyAllowedMethod(const LocationConf &conf) {
  std::string ret = "      Method: ";
  if (conf.allowed_methods_.empty())
    ret += "All";
  else {
    for (std::map<std::string, bool>::const_iterator itr = conf.allowed_methods_.cbegin();
         itr != conf.allowed_methods_.cend(); itr++) {
      if (itr != conf.allowed_methods_.cbegin()) {
        ret += " ";
      }
      ret += itr->first;
    }
  }
  ret += "\n";
  return ret;
}

std::string stringifyLocationConf(const LocationConf &conf) {
  std::string ret = "";
  ret += "      path: " + conf.path_ + "\n";
  ret += stringifyAllowedMethod(conf);
  ret += createStrings("      cgi_ext: ", conf.cgi_exts_);
  ret += stringifyRedirect(conf, 3);
  ret += stringifyCommon(conf.common_, 3);
  return ret;
}

std::string stringifyServConf(const ServerConf &conf) {
  std::string ret = "";
  char buff[100];
  ret += createStrings("    server_name: ", conf.server_names_);
  snprintf(buff, sizeof(buff), "    listen host: %-15s\n", conf.host_.c_str());
  ret += buff;
  snprintf(buff, sizeof(buff), "    port: %d\n", conf.port_);
  ret += buff;
  ret += stringifyRedirect(conf, 2);
  ret += stringifyCommon(conf.common_, 2);
  int i = 0;
  for (std::vector<LocationConf>::const_iterator itr2 = conf.location_confs_.cbegin();
       itr2 != conf.location_confs_.cend(); itr2++, i++) {
    snprintf(buff, sizeof(buff), "    location %d\n", i);
    ret += buff;
    ret += stringifyLocationConf(*itr2);
  }
  return ret;
}

std::string stringifyConfig(const char *filename) {
  Parser parser;
  Config conf = parser.parse(filename);
  std::string ret = "";
  char buff[100];
  snprintf(buff, sizeof(buff), "connection limits: %d\n", conf.limit_connection_);
  ret += buff;
  ret += stringifyCommon(conf.common_, 0);
  int i = 0;
  for (std::vector<ServerConf>::const_iterator itr = conf.server_confs_.cbegin(); itr != conf.server_confs_.cend();
       itr++, i++) {
    snprintf(buff, sizeof(buff), "server %d\n", i);
    ret += buff;
    ret += stringifyServConf(*itr);
  }
  return ret;
}

#if defined(CONFTEST)
TEST(Conftest, OK_01_basic) {
  const std::string ans = readFile("./test/Config/output/OK_01_basic.conf");
  const std::string res = stringifyConfig("./test/Config/input/OK_01_basic.conf");
  EXPECT_EQ(res, ans);
}

TEST(Conftest, OK_02_multiple_value) {
  const std::string ans = readFile("./test/Config/output/OK_02_multiple_value.conf");
  const std::string res = stringifyConfig("./test/Config/input/OK_02_multiple_value.conf");
  EXPECT_EQ(res, ans);
}

TEST(Conftest, OK_03_multiple_location) {
  const std::string ans = readFile("./test/Config/output/OK_03_multiple_location.conf");
  const std::string res = stringifyConfig("./test/Config/input/OK_03_multiple_location.conf");
  EXPECT_EQ(res, ans);
}
#endif