#ifndef CONFIG_HPP_
#define CONFIG_HPP_
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#define MiB 1048576

class CommonConf {
 public:
  CommonConf() : max_body_size_(MiB), root_("/html"), autoindex_(false) {}
  CommonConf(const CommonConf &conf)
      : max_body_size_(conf.max_body_size_),
        root_(conf.root_),
        index_(conf.index_),
        autoindex_(conf.autoindex_),
        error_pages_(conf.error_pages_) {}
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
  void printLocationConf();
  void printAllowedMethod();
  std::map<std::string, bool> &getAllowedMethods();
  std::vector<std::string> &getCGIExtensions();

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
  const LocationConf &getLocationConfig(const std::string &path) const;

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
  const ServerConf *getServerConf(const int port, const std::string &host);
  const LocationConf &getLocationConf(const ServerConf *serv_conf, const std::string &path) const;
  int getLimitConnection() const;

  int limit_connection_;
  CommonConf common_;
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