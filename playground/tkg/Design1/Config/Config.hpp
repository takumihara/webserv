#ifndef CONFIG_HPP_
#define CONFIG_HPP_
#include <map>
#include <string>
#include <vector>

class Config {
 public:
  Config() : connectionlimits_(1024) {}
  class ServerConf {
   public:
    ServerConf() : listen_(8080), server_name_("localhost"), root_("./") {}
    class LocationConf {
     public:
      LocationConf() : root_("html"), path_("/"), index_("index.html") {}

      // private:
      std::string root_;
      std::string path_;
      std::string index_;
    };

    // private:
    int listen_;
    std::string server_name_;
    std::string root_;
    std::vector<LocationConf> location_confs_;
  };

  // private:
  int connectionlimits_;
  std::vector<ServerConf> server_confs_;
};

#endif