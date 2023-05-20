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
      LocationConf() : root_("html"), path_("/") {}
      LocationConf(std::string &path) : path_(path), root_("html") {}

      // private:
      std::string path_;
      std::string root_;
      std::vector<std::string> index_;
    };
    void printServConf();

    // private:
    std::vector<std::string> host_;
    std::vector<int> port_;
    std::vector<std::string> server_names_;
    std::string root_;
    std::vector<std::string> index_;
    std::vector<LocationConf> location_confs_;
    static const int kDefaultPort = 80;
  };
  void makePortServConfMap();
  void printConfig();
  void printPortServConfMap();

  // private:
  int limit_connection_;
  std::vector<ServerConf> server_confs_;
  std::map<int, std::vector<ServerConf *>> port_servConf_map_;
};

#endif