#pragma once

#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

class Config;
class CommonConf;

class Cache {
 public:
  typedef std::map<std::string, std::string> t_map;

  Cache() { initStatusMsg(); }
  void initCache(const Config *conf);

  t_map error_page_paths_;
  std::map<int, std::string> statusMsg_;
  std::map<std::string, std::string> ext_contentType_map_;

 private:
  void cacheErrorPages(const CommonConf *conf);
  void initStatusErrorPageMap(const Config *conf);
  void initStatusMsg();
  void initContentType();
};