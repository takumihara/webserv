#include "Config.hpp"

#include <unistd.h>

#include <iomanip>
#include <ios>
#include <iostream>

#include "../debug.hpp"
#include "../helper.hpp"
#include "HttpRequest.hpp"
#include "validation.h"

void printStrings(const char *prefix, const std::vector<std::string> &strs) {
  if (strs.size() == 0) return;
  std::cout << prefix;
  for (std::vector<std::string>::const_iterator itr = strs.cbegin(); itr != strs.cend(); itr++) {
    std::cout << *itr << " ";
  }
  std::cout << std::endl;
}

// Common class method
// if no index file exists, return empty string
std::string CommonConf::getIndexFile(std::string path) const {
  if (path[path.size() - 1] != '/') path += "/";
  for (std::vector<std::string>::const_iterator itr = index_.cbegin(); itr != index_.cend(); itr++) {
    if (isReadable((path + (*itr)).c_str())) {
      return path + *itr;
    }
  }
  return "";
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

ServerConf *Config::getServerConf(int port, const std::string &host) {
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
void LocationConf::printAllowedMethod() const {
  std::cout << "      Method: ";
  if (allowed_methods_.empty())
    std::cout << "All";
  else {
    for (std::map<std::string, bool>::const_iterator itr = allowed_methods_.cbegin(); itr != allowed_methods_.cend();
         itr++) {
      std::cout << itr->first << " ";
    }
  }
  std::cout << std::endl;
}

std::map<std::string, bool> &LocationConf::getAllowedMethods() { return allowed_methods_; }

std::vector<std::string> &LocationConf::getCGIExtensions() { return cgi_exts_; }

const std::string &LocationConf::getRedirectStatus() const { return redirect_.first; }
const std::string &LocationConf::getRedirectURI() const { return redirect_.second; }

void LocationConf::printLocationConf() const {
  std::cout << "      path: " << this->path_ << std::endl;
  printStrings("      cgi_ext: ", this->cgi_exts_);
  printRedirect(this, 3);
  std::cout << "      max_body_size: " << this->common_.max_body_size_ << std::endl;
  std::cout << "      root: " << this->common_.root_ << std::endl;
  printStrings("      index: ", this->common_.index_);
  printAutoindex(this, 3);
  printAllowedMethod();

  for (std::map<std::string, std::string>::const_iterator itr = common_.error_pages_.cbegin();
       itr != common_.error_pages_.cend(); itr++) {
    std::cout << "      error_status and path: " << itr->first << " " << itr->second << std::endl;
  }
}

LocationConf *ServerConf::getLocationConf(const HttpRequest *req) const {
  const std::string &path = req->getRequestTarget()->getPath();
  const std::string &extension = getExtension(path);
  const bool hasCGI = extension != "";

  std::vector<LocationConf>::const_iterator ret = location_confs_.cbegin();
  size_t match_len = 0;
  for (std::vector<LocationConf>::const_iterator loc_itr = location_confs_.cbegin(); loc_itr != location_confs_.cend();
       loc_itr++) {
    if (!isAcceptableMethod(&(*loc_itr), req->getMethod())) {
      continue;
    }
    if (hasCGI && !contain(loc_itr->cgi_exts_, extension)) {
      continue;
    }
    if (path.find(loc_itr->path_) == 0 && match_len < loc_itr->path_.size()) {
      ret = loc_itr;
      match_len = loc_itr->path_.size();
    }
  }
  return const_cast<LocationConf *>(&(*ret));
}

std::string LocationConf::getTargetPath(const std::string &request_uri) const {
  std::string target = common_.root_;
  if (target[target.size() - 1] == '/' && request_uri[0] == '/') target.erase(target.size() - 1);
  target += request_uri;
  return target;
}

bool LocationConf::hasRedirectDirective() const {
  if (redirect_.first != "") return true;
  return false;
}
