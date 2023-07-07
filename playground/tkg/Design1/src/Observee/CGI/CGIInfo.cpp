#include "CGI/CGIInfo.hpp"

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
    pos = path.find(ext, pos + 1);
  }
  return "";
}

std::string CGIInfo::getCGIWorkingDirectory() { return script_name_.substr(0, script_name_.rfind("/")); }

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

CGIInfo parseCGIInfo(const std::string &path, const std::string &ext, HttpRequest &req, LocationConf *conf) {
  // todo(katakagi):
  std::stringstream ss;
  CGIInfo info;
  info.loc_conf = conf;
  ss << req.headers_.content_length;
  info.content_length_ = ss.str();
  ss.str("");
  ss.clear(std::stringstream::goodbit);
  info.content_type_ = req.headers_.content_type;
  info.gateway_interface_ = "CGI/1.1";
  info.path_info_ = getPathInfo(path, ext);
  info.path_translated_ = setPathTranslated(conf->common_.root_, info.path_info_);
  info.query_string_ = Encoding::unescape(req.request_target_->getRawQuery(), Encoding::QueryComponent);
  // todo(katakagi): set remote_addr and remote_host by value obtained when accept()
  info.request_method_ = methodToString(req.method_);
  info.script_name_ = getScriptName(path, ext);
  info.server_name_ = req.headers_.host.uri_host;
  ss << req.headers_.host.port;
  info.server_port_ = ss.str();
  info.server_protocol_ = "HTTP/1.1";
  info.server_software_ = "Webserv/1.1";
  return info;
}

void deleteEnv(std::vector<char *> &env) {
  for (std::vector<char *>::iterator itr = env.begin(); itr != env.end(); itr++) {
    delete (*itr);
  }
}

void CGIInfo::setEnv(std::vector<char *> &env) {
  env.push_back(strdup(("AUTH_TYPE=" + auth_type_).c_str()));
  env.push_back(strdup(("CONTENT_LENGTH=" + content_length_).c_str()));
  env.push_back(strdup(("CONTENT_TYPE=" + content_type_).c_str()));
  env.push_back(strdup(("GATEWAT_INTERFACE=" + gateway_interface_).c_str()));
  env.push_back(strdup(("PATH_INFO=" + path_info_).c_str()));
  env.push_back(strdup(("PATH_TRANSLATED=" + path_translated_).c_str()));
  env.push_back(strdup(("QUERY_STRING=" + query_string_).c_str()));
  env.push_back(strdup(("REMOTE_ADDR=" + remote_addr_).c_str()));
  env.push_back(strdup(("REMOTE_HOST=" + remote_host_).c_str()));
  env.push_back(strdup(("REMOET_IDENT=" + remote_ident_).c_str()));
  env.push_back(strdup(("REMOTE_USER=" + remote_user_).c_str()));
  env.push_back(strdup(("REQUEST_METHOD=" + request_method_).c_str()));
  env.push_back(strdup(("SCRIPT_NAME=" + script_name_).c_str()));
  env.push_back(strdup(("SERVER_NAME=" + server_name_).c_str()));
  env.push_back(strdup(("SERVER_PORT=" + server_port_).c_str()));
  env.push_back(strdup(("SERVER_PROTOCOL=" + server_protocol_).c_str()));
  env.push_back(strdup(("SERVER_SOFTWARE=" + server_software_).c_str()));
  env.push_back(strdup(("PROTOCOL_VAR_NAME=" + protocol_var_name_).c_str()));
  env.push_back(strdup(("EXTENSION_VAR_NAME=" + extension_var_name_).c_str()));
  env.push_back(NULL);
}
