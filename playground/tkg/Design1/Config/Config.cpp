#include "Config.hpp"

#include <iomanip>
#include <ios>
#include <iostream>

void printStrings(const char *prefix, std::vector<std::string> &strs) {
  if (strs.size() == 0) return;
  std::cout << prefix;
  for (std::vector<std::string>::iterator itr = strs.begin(); itr != strs.end(); itr++) {
    std::cout << *itr << " ";
  }
  std::cout << std::endl;
}

void Config::makePortServConfMap() {
  for (std::vector<ServerConf>::iterator serv_itr = server_confs_.begin(); serv_itr != server_confs_.end();
       serv_itr++) {
    for (std::vector<int>::iterator port_itr = serv_itr->port_.begin(); port_itr != serv_itr->port_.end(); port_itr++) {
      (port_servConf_map_[*port_itr]).push_back(&(*serv_itr));
    }
  }
}

void Config::printConfig() {
  std::cout << "connection limits: " << limit_connection_ << std::endl;
  printAutoindex(this, 0);
  for (std::map<std::string, std::string>::iterator itr = error_pages_.begin(); itr != error_pages_.end(); itr++) {
    std::cout << "error_status and path: " << itr->first << " " << itr->second << std::endl;
  }
  int i = 0;
  for (std::vector<ServConf>::iterator itr = server_confs_.begin(); itr != server_confs_.end(); itr++, i++) {
    std::cout << "server" << i << std::endl;
    itr->printServConf();
  }
}

void Config::ServerConf::printServConf() {
  std::cout << "  server" << std::endl;
  printStrings("    server_name: ", server_names_);
  printStrings("    index: ", index_);
  printAutoindex(this, 2);
  for (std::map<std::string, std::string>::iterator itr = error_pages_.begin(); itr != error_pages_.end(); itr++) {
    std::cout << "    error_status and path: " << itr->first << " " << itr->second << std::endl;
  }
  for (size_t i = 0; i < host_.size(); i++) {
    std::cout << "    listen host: " << std::setw(15) << std::left << host_[i];
    std::cout << "    port: " << port_[i] << std::endl;
  }
  int j = 0;
  for (std::vector<LocConf>::iterator itr2 = location_confs_.begin(); itr2 != location_confs_.end(); itr2++, j++) {
    std::cout << "    location" << j << std::endl;
    itr2->printLocationConf();
  }
}

void Config::ServerConf::LocationConf::printLocationConf() {
  std::cout << "      path: " << this->path_ << std::endl;
  std::cout << "      root: " << this->root_ << std::endl;
  printStrings("      index: ", this->index_);
  printAutoindex(this, 3);
  for (std::map<std::string, std::string>::iterator itr = error_pages_.begin(); itr != error_pages_.end(); itr++) {
    std::cout << "      error_status and path: " << itr->first << " " << itr->second << std::endl;
  }
}

void Config::printPortServConfMap() {
  for (std::map<int, std::vector<ServerConf *> >::iterator map_itr = port_servConf_map_.begin();
       map_itr != port_servConf_map_.end(); map_itr++) {
    std::cout << "port: " << map_itr->first << std::endl;
    for (std::vector<ServerConf *>::iterator serv_itr = map_itr->second.begin(); serv_itr != map_itr->second.end();
         serv_itr++) {
      (*serv_itr)->printServConf();
    }
  }
}

const Config::ServerConf *Config::getServConfig(int port, const std::string &host) {
  std::vector<ServConf *> servs = port_servConf_map_[port];
  for (std::vector<ServerConf *>::iterator serv_itr = servs.begin(); serv_itr != servs.end(); serv_itr++) {
    std::vector<std::string> &names = (*serv_itr)->getServerNames();
    for (std::vector<std::string>::iterator name_itr = names.begin(); name_itr != names.end(); name_itr++) {
      if (*name_itr == host) return *serv_itr;
    }
  }
  return servs[0];
}

const Config::ServerConf::LocationConf &Config::getLocationConfig(const Config::ServerConf *serv_conf,
                                                                  const std::string &path) const {
  const std::vector<Config::ServerConf::LocationConf> &locs = serv_conf->location_confs_;
  std::vector<LocConf>::const_iterator ret = locs.begin();
  size_t match_len = 0;
  for (std::vector<LocConf>::const_iterator loc_itr = locs.begin(); loc_itr != locs.end(); loc_itr++) {
    if (path.find(loc_itr->path_) == 0 && match_len < loc_itr->path_.size()) {
      ret = loc_itr;
      match_len = loc_itr->path_.size();
    }
  }
  return *ret;
}

int Config::getLimitConnection() const { return limit_connection_; }

std::vector<std::string> &Config::ServerConf::getHostNames() { return host_; }

std::vector<int> &Config::ServerConf::getPorts() { return port_; }

std::vector<std::string> &Config::ServerConf::getServerNames() { return server_names_; }
