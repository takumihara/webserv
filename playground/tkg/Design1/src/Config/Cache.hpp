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
  typedef std::map<std::string, std::string *> t_map_star;
  Cache() { initStatusMsg(); }
  void cacheErrorPages(const CommonConf *conf);
  void initStatusErrorPageMap(const Config *conf);
  void initStatusMsg();

  t_map error_page_paths_;
  t_map_star status_errorPage_map_;
  std::map<int, std::string> statusMsg_;
};
