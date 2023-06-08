#ifndef CONFIG_HPP_
#define CONFIG_HPP_
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "Cache.hpp"

#define MiB 1048576

class HttpRequest;

class CommonConf {
 public:
  CommonConf() : max_body_size_(MiB), root_("/html"), autoindex_(false) {}
  CommonConf(const CommonConf &conf)
      : max_body_size_(conf.max_body_size_),
        root_(conf.root_),
        index_(conf.index_),
        autoindex_(conf.autoindex_),
        error_pages_(conf.error_pages_) {}
  bool operator==(const CommonConf &rhs) const {
    bool flag = true;
    flag &= max_body_size_ == rhs.max_body_size_;
    flag &= root_ == rhs.root_;
    flag &= index_ == rhs.index_;
    flag &= autoindex_ == rhs.autoindex_;
    flag &= error_pages_ == rhs.error_pages_;
    return flag;
  }
  std::string getIndexFile(std::string path) const;
  std::size_t max_body_size_;
  std::string root_;
  std::vector<std::string> index_;
  bool autoindex_;
  std::map<std::string, std::string> error_pages_;
};

class LocationConf {
 public:
  LocationConf(const std::string &path, const std::pair<std::string, std::string> redir, const CommonConf &conf)
      : path_(path), redirect_(redir), common_(conf) {}
  bool operator==(const LocationConf &rhs) const {
    bool flag = true;
    flag &= path_ == rhs.path_;
    flag &= redirect_ == rhs.redirect_;
    flag &= allowed_methods_ == rhs.allowed_methods_;
    flag &= cgi_exts_ == rhs.cgi_exts_;
    flag &= common_ == rhs.common_;
    return flag;
  }
  void printLocationConf() const;
  void printAllowedMethod() const;
  std::map<std::string, bool> &getAllowedMethods();
  std::vector<std::string> &getCGIExtensions();
  const std::string &getRedirectStatus() const;
  const std::string &getRedirectURI() const;
  std::string getTargetPath(const std::string &request_uri) const;
  bool hasRedirectDirective() const;

  std::string path_;
  std::pair<std::string, std::string> redirect_;
  CommonConf common_;
  std::map<std::string, bool> allowed_methods_;
  std::vector<std::string> cgi_exts_;
};  // Location Config

class ServerConf {
 public:
  ServerConf(const CommonConf &conf) : common_(conf) {}
  void printServConf();
  std::vector<std::string> &getServerNames();
  std::string &getHostNames();
  int &getPorts();
  LocationConf *getLocationConf(const HttpRequest *req);
  std::string host_;
  int port_;
  std::vector<std::string> server_names_;
  std::pair<std::string, std::string> redirect_;
  CommonConf common_;
  std::vector<LocationConf> location_confs_;
};

class Config {
 public:
  Config() : limit_connection_(1024) {}
  Config(const Config &conf)
      : limit_connection_(conf.limit_connection_),
        common_(conf.common_),
        server_confs_(conf.server_confs_),
        port_servConf_map_(conf.port_servConf_map_) {}
  void makePortServConfMap();

  void printConfig();
  void printPortServConfMap();
  ServerConf *getServerConf(const int port, const std::string &host);
  int getLimitConnection() const;
  int getMaxBodySize() const;

  int limit_connection_;
  CommonConf common_;
  Cache cache_;
  std::vector<ServerConf> server_confs_;
  std::map<int, std::vector<ServerConf *> > port_servConf_map_;
};

template <class T>
void printAutoindex(T conf, int indent) {
  for (int i = 0; i < indent; i++) {
    std::cout << "  ";
  }
  std::cout << "autoindex: ";
  if (conf->common_.autoindex_)
    std::cout << "on" << std::endl;
  else
    std::cout << "off" << std::endl;
}

template <class T>
void printRedirect(T conf, int indent) {
  for (int i = 0; i < indent; i++) {
    std::cout << "  ";
  }
  std::cout << "redirect: "
            << "code: " << conf->redirect_.first << "  to: " << conf->redirect_.second << std::endl;
}

template <class T>
std::vector<std::string> &getIndex(T conf) {
  return conf->common_.index_;
}

template <class T>
std::string &getRoot(T conf) {
  return conf->common_.root_;
}

template <class T>
std::map<std::string, std::string> &getErrorPage(T conf) {
  return conf->common_.error_pages_;
}

template <class T>
bool getAutoindex(T conf) {
  return conf->common_.autoindex_;
}

template <class T>
std::size_t getMaxBodySize(T conf) {
  return conf->common_.max_body_size_;
}

template <class T>
std::pair<std::string, std::string> &getRedirect(T conf) {
  return conf->redirect_;
}

#endif
