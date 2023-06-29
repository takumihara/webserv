#pragma once

#include "../Config/Config.hpp"
#include "../HttpRequest/HttpRequest.hpp"
struct CGIInfo {
  LocationConf *loc_conf;
  std::string auth_type_;
  std::string content_length_;
  std::string content_type_;
  std::string gateway_interface_;
  std::string path_info_;
  std::string path_translated_;
  std::string query_string_;
  std::string remote_addr_;
  std::string remote_host_;
  std::string remote_ident_;
  std::string remote_user_;
  std::string request_method_;
  std::string script_name_;
  std::string server_name_;
  std::string server_port_;
  std::string server_protocol_;
  std::string server_software_;
  std::string protocol_var_name_;
  std::string extension_var_name_;
  std::string cookie_;

  void setEnv(std::vector<char *> &env);
};

std::string getPathInfo(const std::string &path, const std::string &ext);
std::string getScriptName(const std::string &path, const std::string &ext);
CGIInfo parseCGIInfo(const std::string &path, const std::string &ext, HttpRequest &req, LocationConf *conf);
std::string setPathTranslated(std::string root, std::string &path_info);
void deleteEnv(std::vector<char *> &env);
