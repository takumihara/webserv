#ifndef CONFIG_HPP_
#define CONFIG_HPP_
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

class Config {
 public:
  Config() : limit_connection_(1024), autoindex_(false) {}
  class ServerConf {
   public:
    ServerConf(Config &conf)
        : root_(conf.root_), index_(conf.index_), autoindex_(conf.autoindex_), error_pages_(conf.error_pages_) {}
    class LocationConf {
     public:
      LocationConf(std::string &path, ServerConf &conf)
          : path_(path),
            root_(conf.root_),
            index_(conf.index_),
            autoindex_(conf.autoindex_),
            error_pages_(conf.error_pages_) {}
      void printLocationConf();
      // private:
      std::string path_;
      std::string root_;
      std::vector<std::string> index_;
      bool autoindex_;
      std::map<std::string, std::string> error_pages_;
    };
    void printServConf();
    std::vector<std::string> &getServerNames();
    std::vector<std::string> &getHostNames();
    std::vector<int> &getPorts();

    // private:
    std::vector<std::string> host_;
    std::vector<int> port_;
    std::vector<std::string> server_names_;
    std::string root_;
    std::vector<std::string> index_;
    bool autoindex_;
    std::map<std::string, std::string> error_pages_;
    std::vector<LocationConf> location_confs_;
  };
  typedef Config::ServerConf ServConf;
  typedef Config::ServerConf::LocationConf LocConf;

  void makePortServConfMap();
  void printConfig();
  void printPortServConfMap();
  const ServerConf *getServConfig(int port, const std::string &host);
  const LocConf &getLocationConfig(const ServerConf *serv_conf, const std::string &path) const;
  int getLimitConnection() const;
  // private:
  int limit_connection_;
  std::string root_;
  std::vector<std::string> index_;
  bool autoindex_;
  std::map<std::string, std::string> error_pages_;
  std::vector<ServerConf> server_confs_;
  std::map<int, std::vector<ServerConf *> > port_servConf_map_;
};

template <class T>
void printAutoindex(T conf, int indent) {
  for (int i = 0; i < indent; i++) {
    std::cout << "  ";
  }
  std::cout << "autoindex: ";
  if (conf->autoindex_)
    std::cout << "on" << std::endl;
  else
    std::cout << "off" << std::endl;
}

template <class T>
std::vector<std::string> &getIndex(T conf) {
  return conf->index_;
}

template <class T>
std::string &getRoot(T conf) {
  return conf->root_;
}

#endif
