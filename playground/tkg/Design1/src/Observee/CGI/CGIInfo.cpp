#include "CGIInfo.hpp"

#include <cstdlib>
#include <sstream>

#include "../../helper.hpp"
#include "Config/validation.h"
#include "HttpRequest/HttpRequest.hpp"
#include "URI/URI.hpp"
#include "URI/encoding.hpp"

std::string getScriptName(const std::string &path, const std::string &ext) {
  std::size_t pos = path.find(ext);
  while (pos != std::string::npos) {
    if (path.size() == pos + ext.size())
      return path;
    else if (path[pos + ext.size()] == '/' || path[pos + ext.size()] == ';') {
      return path.substr(0, pos + ext.size());
    }
  }
  return "";
}

std::string getPathInfo(const std::string &path, const std::string &ext) {
  std::size_t pos = path.find(ext);
  while (pos != std::string::npos) {
    if (path.size() == pos + ext.size())
      return "";
    else if (path[pos + ext.size()] == '/' || path[pos + ext.size()] == ';') {
      std::string path_info = path.substr(pos + ext.size());
      if (path_info[0] == ';') path_info[0] = '/';
      if (path_info[0] != '/') path_info = "/" + path_info;
      std::cout << "path_info: " << path_info << std::endl;
      return path_info;
    }
    pos = path.find(ext, pos + 1);
  }
  return "";
}

std::string setPathTranslated(std::string root, std::string &path_info) {
  if (root == "/") return path_info;
  if (root[root.size() - 1] == '/') root.pop_back();
  return root + path_info;
}

CGIInfo parseCGI(const std::string &path, const std::string &ext, HttpRequest &req, LocationConf *conf) {
  // todo:
  std::stringstream ss;
  CGIInfo info;
  info.loc_conf = conf;
  ss << req.headers_.content_length;
  info.content_length_ = ss.str();
  ss.str("");
  ss.clear(std::stringstream::goodbit);
  info.content_type_ = "text/plain";
  info.gateway_interface_ = "CGI/1.1";
  info.path_info_ = getPathInfo(path, ext);
  info.path_translated_ = setPathTranslated(conf->common_.root_, info.path_info_);
  info.query_string_ = Encoding::unescape(req.getRequestTarget()->getRawQuery(), Encoding::QueryComponent);
  // todo: set remote_addr and remote_host by value obtained when accept()
  info.request_method_ = methodToString(req.method_);
  info.script_name_ = getScriptName(path, ext);
  info.server_name_ = req.getHost().uri_host;
  ss << req.getHost().port;
  info.server_port_ = ss.str();
  info.server_protocol_ = "HTTP/1.1";
  info.server_software_ = "Webserv/1.1";
  return info;
}

void setCGIInfo(CGIInfo &info) {
  setenv("AUTH_TYPE", info.auth_type_.c_str(), 1);
  setenv("CONTENT_LENGTH", info.content_length_.c_str(), 1);
  setenv("CONTENT_TYPE", info.content_type_.c_str(), 1);
  setenv("GATEWAT_INTERFACE", info.gateway_interface_.c_str(), 1);
  setenv("PATH_INFO", info.path_info_.c_str(), 1);
  setenv("PATH_TRANSLATED", info.path_translated_.c_str(), 1);
  setenv("QUERY_STRING", info.query_string_.c_str(), 1);
  setenv("REMOTE_ADDR", info.remote_addr_.c_str(), 1);
  setenv("REMOTE_HOST", info.remote_host_.c_str(), 1);
  setenv("REMOET_IDENT", info.remote_ident_.c_str(), 1);
  setenv("REMOTE_USER", info.remote_user_.c_str(), 1);
  setenv("REQUEST_METHOD", info.request_method_.c_str(), 1);
  setenv("SCRIPT_NAME", info.script_name_.c_str(), 1);
  setenv("SERVER_NAME", info.server_name_.c_str(), 1);
  setenv("SERVER_PORT", info.server_port_.c_str(), 1);
  setenv("SERVER_PROTOCOL", info.server_protocol_.c_str(), 1);
  setenv("SERVER_SOFTWARE", info.server_software_.c_str(), 1);
  setenv("PROTOCOL_VAR_NAME", info.protocol_var_name_.c_str(), 1);
  setenv("EXTENSION_VAR_NAME", info.extension_var_name_.c_str(), 1);
}