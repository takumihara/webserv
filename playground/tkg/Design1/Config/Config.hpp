#ifndef CONFIG_HPP_
#define CONFIG_HPP_
#include <map>
#include <string>
#include <vector>

class Config {
 public:
  Config() : limit_connection_(1024) {}
  class ServerConf {
   public:
    ServerConf() : root_("./") {}
    class LocationConf {
     public:
      LocationConf() : path_("/"), root_("html") {}
      LocationConf(std::string &path, std::string root) : path_(path), root_(root) {}

      // private:
      std::string path_;
      std::string root_;
      std::vector<std::string> index_;
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
    std::vector<LocationConf> location_confs_;
  };
  typedef Config::ServerConf ServConf;
  typedef Config::ServerConf::LocationConf LocConf;

  void makePortServConfMap();
  void printConfig();
  void printPortServConfMap();
  const ServerConf *getServConfig(int port, std::string &host);
  const LocConf &getLocationConfig(ServerConf *serv_conf, std::string &path) const;
  int getLimitConnection() const;
  // private:
  int limit_connection_;
  std::vector<ServerConf> server_confs_;
  std::map<int, std::vector<ServerConf *> > port_servConf_map_;
};

template <class T>
std::vector<std::string> &getIndex(T conf) {
  return conf->index_;
}

template <class T>
std::string &getRoot(T conf) {
  return conf->root_;
}

#endif