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
  void initCache(Config *conf);

  t_map error_page_paths_;
  std::map<int, std::string> statusMsg_;
  std::map<std::string, std::string> ext_contentType_map_;

 private:
  void cacheErrorPages(CommonConf *conf);
  void initStatusErrorPageMap(Config *conf);
  void initStatusMsg();
  void initContentType();
};
