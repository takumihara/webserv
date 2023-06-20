#include "Cache.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "Config.hpp"

static std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void Cache::initCache(const Config *conf) {
  initStatusMsg();
  initStatusErrorPageMap(conf);
}

void Cache::cacheErrorPages(const CommonConf *conf) {
  const std::map<std::string, std::string> &error_pages = conf->error_pages_;
  const std::string &root = conf->root_;
  std::string file;
  for (t_map::const_iterator itr = error_pages.cbegin(); itr != error_pages.cend(); itr++) {
    file = itr->second;
    // translate into abs-path when error page path is relative-path
    if (itr->second[0] != '/') file = root + "/" + file;
    if (error_page_paths_.find(file) == error_page_paths_.end()) {
      std::cerr << file << std::endl;
      error_page_paths_[file] = readFile(("." + file).c_str());
    }
  }
}

void Cache::initStatusErrorPageMap(const Config *conf) {
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
  // 2xx
  statusMsg_.insert(std::make_pair(200, "OK"));
  statusMsg_.insert(std::make_pair(201, "Created"));
  statusMsg_.insert(std::make_pair(202, "Accepted"));
  statusMsg_.insert(std::make_pair(203, "Non-Authoritative Information"));
  statusMsg_.insert(std::make_pair(204, "No Content"));
  statusMsg_.insert(std::make_pair(205, "Reset Content"));
  statusMsg_.insert(std::make_pair(206, "Partial Content"));
  statusMsg_.insert(std::make_pair(207, "Multi-Status"));
  statusMsg_.insert(std::make_pair(208, "Already Reported"));
  statusMsg_.insert(std::make_pair(226, "IM Used"));
  // 3xx
  statusMsg_.insert(std::make_pair(300, "Multiple Choices"));
  statusMsg_.insert(std::make_pair(301, "Moved Permanently"));
  statusMsg_.insert(std::make_pair(302, "Found"));
  statusMsg_.insert(std::make_pair(303, "See Other"));
  statusMsg_.insert(std::make_pair(304, "Not Modified"));
  statusMsg_.insert(std::make_pair(305, "Use Proxy"));
  statusMsg_.insert(std::make_pair(307, "Temporary Redirect"));
  statusMsg_.insert(std::make_pair(308, "Permanent Redirect"));
  // 4xx
  statusMsg_.insert(std::make_pair(400, "Bad Request"));
  statusMsg_.insert(std::make_pair(401, "Unauthorized"));
  statusMsg_.insert(std::make_pair(402, "Payment Required"));
  statusMsg_.insert(std::make_pair(403, "Forbidden"));
  statusMsg_.insert(std::make_pair(404, "Not Found"));
  statusMsg_.insert(std::make_pair(405, "Method Not Allowed"));
  statusMsg_.insert(std::make_pair(406, "Not Acceptable"));
  statusMsg_.insert(std::make_pair(407, "Proxy Authentication Required"));
  statusMsg_.insert(std::make_pair(408, "Request Timeout"));
  statusMsg_.insert(std::make_pair(409, "Conflict"));
  statusMsg_.insert(std::make_pair(410, "Gone"));
  statusMsg_.insert(std::make_pair(411, "Length Required"));
  statusMsg_.insert(std::make_pair(412, "Precondition Failed"));
  statusMsg_.insert(std::make_pair(413, "Payload Too Large"));
  statusMsg_.insert(std::make_pair(414, "URI Too Long"));
  statusMsg_.insert(std::make_pair(415, "Unsupported Media Type"));
  statusMsg_.insert(std::make_pair(416, "Range Not Satisfiable"));
  statusMsg_.insert(std::make_pair(417, "Expectation Failed"));
  statusMsg_.insert(std::make_pair(418, "I'm a teapot"));
  statusMsg_.insert(std::make_pair(421, "Misdirected Request"));
  statusMsg_.insert(std::make_pair(422, "Unprocessable Entity"));
  statusMsg_.insert(std::make_pair(423, "Locked"));
  statusMsg_.insert(std::make_pair(424, "Failed Dependency"));
  statusMsg_.insert(std::make_pair(425, "Too Early"));
  statusMsg_.insert(std::make_pair(426, "Upgrade Required"));
  statusMsg_.insert(std::make_pair(428, "Precondition Required"));
  statusMsg_.insert(std::make_pair(429, "Too Many Requests"));
  statusMsg_.insert(std::make_pair(431, "Request Header Fields Too Large"));
  statusMsg_.insert(std::make_pair(451, "Unavailable For Legal Reasons"));
  // 5xx
  statusMsg_.insert(std::make_pair(500, "Internal Server Error"));
  statusMsg_.insert(std::make_pair(501, "Not Implemented"));
  statusMsg_.insert(std::make_pair(502, "Bad Gateway"));
  statusMsg_.insert(std::make_pair(503, "Service Unavailable"));
  statusMsg_.insert(std::make_pair(504, "Gateway Timeout"));
  statusMsg_.insert(std::make_pair(505, "HTTP Version Not Supported"));
  statusMsg_.insert(std::make_pair(506, "Variant Also Negotiates"));
  statusMsg_.insert(std::make_pair(507, "Insufficient Storage"));
  statusMsg_.insert(std::make_pair(508, "Loop Detected"));
  statusMsg_.insert(std::make_pair(510, "Not Extended"));
  statusMsg_.insert(std::make_pair(511, "Network Authentication Required"));
}