#include "Cache.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "Config.hpp"

static std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("conf file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void Cache::cacheErrorPages(const CommonConf *conf) {
  std::map<std::string, std::string> error_pages = conf->error_pages_;
  std::string root = conf->root_;
  std::string key;
  std::string file;
  for (t_map::const_iterator itr = error_pages.cbegin(); itr != error_pages.cend(); itr++) {
    key = itr->first;
    file = itr->second;
    if (itr->second[0] != '/') file = root + "/" + file;
    key += file;
    if (status_errorPage_map_.find(key) != status_errorPage_map_.end()) continue;
    if (error_page_paths_.find(file) == error_page_paths_.end()) {
      std::cerr << file << std::endl;
      std::cerr << key << std::endl;
      error_page_paths_[file] = readFile(("." + file).c_str());
    }
    status_errorPage_map_[key] = &error_page_paths_[file];
  }
}

void Cache::initStatusErrorPageMap(const Config *conf) {
  std::map<std::string, bool> error_page_paths;
  CommonConf common = conf->common_;
  cacheErrorPages(&common);
  for (std::vector<ServerConf>::const_iterator serv = conf->server_confs_.cbegin(); serv != conf->server_confs_.cend();
       serv++) {
    cacheErrorPages(&serv->common_);
    for (std::vector<LocationConf>::const_iterator loc = serv->location_confs_.cbegin();
         loc != serv->location_confs_.cend(); loc++) {
      cacheErrorPages(&loc->common_);
    }
  }
  return;
}

void Cache::initStatusMsg() {
  statusMsg_.insert(std::make_pair(400, std::string("Bad Request")));
  statusMsg_.insert(std::make_pair(403, std::string("Forbiden")));
  statusMsg_.insert(std::make_pair(404, std::string("Not Found")));
  statusMsg_.insert(std::make_pair(405, std::string("Method Not Allowed")));
  statusMsg_.insert(std::make_pair(500, std::string("Internal Server Error")));
  statusMsg_.insert(std::make_pair(501, std::string("Not Implemented")));
  statusMsg_.insert(std::make_pair(505, std::string("HTTP Version Not Supported")));
}