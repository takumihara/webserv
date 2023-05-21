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
      LocationConf(std::string &path) : path_(path), root_("html") {}

      // private:
      std::string path_;
      std::string root_;
      std::vector<std::string> index_;
    };
    void printServConf();
    std::vector<std::string> &getServerName();

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
  ServerConf *getServConfig(int port, std::string &host);
  LocConf &getLocationConfig(ServerConf *serv_conf, std::string &path);
  // private:
  int limit_connection_;
  std::vector<ServerConf> server_confs_;
  std::map<int, std::vector<ServerConf *> > port_servConf_map_;
};

#endif