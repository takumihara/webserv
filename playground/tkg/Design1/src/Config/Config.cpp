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

// Config class method
void Config::makePortServConfMap() {
  for (std::vector<ServerConf>::iterator serv_itr = server_confs_.begin(); serv_itr != server_confs_.end();
       serv_itr++) {
    (port_servConf_map_[serv_itr->port_]).push_back(&(*serv_itr));
  }
}

void Config::printConfig() {
  std::cout << "connection limits: " << limit_connection_ << std::endl;
  std::cout << "max_body_size: " << common_.max_body_size_ << std::endl;
  printAutoindex(this, 0);

  for (std::map<std::string, std::string>::iterator itr = common_.error_pages_.begin();
       itr != common_.error_pages_.end(); itr++) {
    std::cout << "error_status and path: " << itr->first << " " << itr->second << std::endl;
  }
  int i = 0;
  for (std::vector<ServerConf>::iterator itr = server_confs_.begin(); itr != server_confs_.end(); itr++, i++) {
    std::cout << "server" << i << std::endl;
    itr->printServConf();
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

const ServerConf *Config::getServerConf(int port, const std::string &host) {
  std::vector<ServerConf *> servs = port_servConf_map_[port];
  for (std::vector<ServerConf *>::iterator serv_itr = servs.begin(); serv_itr != servs.end(); serv_itr++) {
    std::vector<std::string> &names = (*serv_itr)->getServerNames();
    for (std::vector<std::string>::iterator name_itr = names.begin(); name_itr != names.end(); name_itr++) {
      if (*name_itr == host) return *serv_itr;
    }
  }
  return servs[0];
}

int Config::getLimitConnection() const { return limit_connection_; }

// todo: provide the most suitable one
int Config::getMaxBodySize() const { return common_.max_body_size_; }

// ServerConfig class method
void ServerConf::printServConf() {
  std::cout << "  server" << std::endl;
  printStrings("    server_name: ", server_names_);
  printRedirect(this, 2);
  std::cout << "    max_body_size: " << common_.max_body_size_ << std::endl;
  printStrings("    index: ", common_.index_);
  printAutoindex(this, 2);
  for (std::map<std::string, std::string>::iterator itr = common_.error_pages_.begin();
       itr != common_.error_pages_.end(); itr++) {
    std::cout << "    error_status and path: " << itr->first << " " << itr->second << std::endl;
  }
  std::cout << "    listen host: " << std::setw(15) << std::left << host_;
  std::cout << "    port: " << port_ << std::endl;
  int j = 0;
  for (std::vector<LocationConf>::iterator itr2 = location_confs_.begin(); itr2 != location_confs_.end(); itr2++, j++) {
    std::cout << "    location" << j << std::endl;
    itr2->printLocationConf();
  }
}

std::string &ServerConf::getHostNames() { return host_; }

int &ServerConf::getPorts() { return port_; }

std::vector<std::string> &ServerConf::getServerNames() { return server_names_; }

// LocationConf class method
void LocationConf::printAllowedMethod() {
  std::cout << "      Method: ";
  if (allowed_methods_.empty())
    std::cout << "All";
  else {
    for (std::map<std::string, bool>::iterator itr = allowed_methods_.begin(); itr != allowed_methods_.end(); itr++) {
      std::cout << itr->first << " ";
    }
  }
  std::cout << std::endl;
}

std::map<std::string, bool> &LocationConf::getAllowedMethods() { return allowed_methods_; }

std::vector<std::string> &LocationConf::getCGIExtensions() { return cgi_exts_; }

void LocationConf::printLocationConf() {
  std::cout << "      path: " << this->path_ << std::endl;
  printStrings("      cgi_ext: ", this->cgi_exts_);
  printRedirect(this, 3);
  std::cout << "      max_body_size: " << this->common_.max_body_size_ << std::endl;
  std::cout << "      root: " << this->common_.root_ << std::endl;
  printStrings("      index: ", this->common_.index_);
  printAutoindex(this, 3);
  printAllowedMethod();

  for (std::map<std::string, std::string>::iterator itr = common_.error_pages_.begin();
       itr != common_.error_pages_.end(); itr++) {
    std::cout << "      error_status and path: " << itr->first << " " << itr->second << std::endl;
  }
}

const LocationConf &Config::getLocationConf(const ServerConf *serv_conf, const std::string &path) const {
  const std::vector<LocationConf> &locs = serv_conf->location_confs_;
  std::vector<LocationConf>::const_iterator ret = locs.begin();
  size_t match_len = 0;
  for (std::vector<LocationConf>::const_iterator loc_itr = locs.begin(); loc_itr != locs.end(); loc_itr++) {
    if (path.find(loc_itr->path_) == 0 && match_len < loc_itr->path_.size()) {
      ret = loc_itr;
      match_len = loc_itr->path_.size();
    }
  }
  return *ret;
}
