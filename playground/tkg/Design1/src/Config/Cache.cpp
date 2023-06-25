#include "Cache.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "Config.hpp"

static std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

void Cache::initCache(const Config *conf) {
  initStatusMsg();
  initStatusErrorPageMap(conf);
}

void Cache::cacheErrorPages(const CommonConf *conf) {
  const std::map<std::string, std::string> &error_pages = conf->error_pages_;
  const std::string &root = conf->root_;
  std::string file;
  for (t_map::const_iterator itr = error_pages.cbegin(); itr != error_pages.cend(); itr++) {
    file = itr->second;
    // translate into abs-path when error page path is relative-path
    if (itr->second[0] != '/') file = root + "/" + file;
    if (error_page_paths_.find(file) == error_page_paths_.end()) {
      std::cerr << file << std::endl;
      error_page_paths_[file] = readFile(("." + file).c_str());
    }
  }
}

void Cache::initStatusErrorPageMap(const Config *conf) {
  CommonConf common = conf->common_;
  cacheErrorPages(&common);
  for (std::vector<ServerConf>::const_iterator serv = conf->server_confs_.cbegin(); serv != conf->server_confs_.cend();
       serv++) {
    cacheErrorPages(&serv->common_);
    for (std::vector<LocationConf>::const_iterator loc = serv->location_confs_.cbegin();
         loc != serv->location_confs_.cend(); loc++) {
      cacheErrorPages(&loc->common_);
    }
  }
  return;
}

void Cache::initStatusMsg() {
  // 2xx
  statusMsg_.insert(std::make_pair(200, "OK"));
  statusMsg_.insert(std::make_pair(201, "Created"));
  statusMsg_.insert(std::make_pair(202, "Accepted"));
  statusMsg_.insert(std::make_pair(203, "Non-Authoritative Information"));
  statusMsg_.insert(std::make_pair(204, "No Content"));
  statusMsg_.insert(std::make_pair(205, "Reset Content"));
  statusMsg_.insert(std::make_pair(206, "Partial Content"));
  statusMsg_.insert(std::make_pair(207, "Multi-Status"));
  statusMsg_.insert(std::make_pair(208, "Already Reported"));
  statusMsg_.insert(std::make_pair(226, "IM Used"));
  // 3xx
  statusMsg_.insert(std::make_pair(300, "Multiple Choices"));
  statusMsg_.insert(std::make_pair(301, "Moved Permanently"));
  statusMsg_.insert(std::make_pair(302, "Found"));
  statusMsg_.insert(std::make_pair(303, "See Other"));
  statusMsg_.insert(std::make_pair(304, "Not Modified"));
  statusMsg_.insert(std::make_pair(305, "Use Proxy"));
  statusMsg_.insert(std::make_pair(307, "Temporary Redirect"));
  statusMsg_.insert(std::make_pair(308, "Permanent Redirect"));
  // 4xx
  statusMsg_.insert(std::make_pair(400, "Bad Request"));
  statusMsg_.insert(std::make_pair(401, "Unauthorized"));
  statusMsg_.insert(std::make_pair(402, "Payment Required"));
  statusMsg_.insert(std::make_pair(403, "Forbidden"));
  statusMsg_.insert(std::make_pair(404, "Not Found"));
  statusMsg_.insert(std::make_pair(405, "Method Not Allowed"));
  statusMsg_.insert(std::make_pair(406, "Not Acceptable"));
  statusMsg_.insert(std::make_pair(407, "Proxy Authentication Required"));
  statusMsg_.insert(std::make_pair(408, "Request Timeout"));
  statusMsg_.insert(std::make_pair(409, "Conflict"));
  statusMsg_.insert(std::make_pair(410, "Gone"));
  statusMsg_.insert(std::make_pair(411, "Length Required"));
  statusMsg_.insert(std::make_pair(412, "Precondition Failed"));
  statusMsg_.insert(std::make_pair(413, "Payload Too Large"));
  statusMsg_.insert(std::make_pair(414, "URI Too Long"));
  statusMsg_.insert(std::make_pair(415, "Unsupported Media Type"));
  statusMsg_.insert(std::make_pair(416, "Range Not Satisfiable"));
  statusMsg_.insert(std::make_pair(417, "Expectation Failed"));
  statusMsg_.insert(std::make_pair(418, "I'm a teapot"));
  statusMsg_.insert(std::make_pair(421, "Misdirected Request"));
  statusMsg_.insert(std::make_pair(422, "Unprocessable Entity"));
  statusMsg_.insert(std::make_pair(423, "Locked"));
  statusMsg_.insert(std::make_pair(424, "Failed Dependency"));
  statusMsg_.insert(std::make_pair(425, "Too Early"));
  statusMsg_.insert(std::make_pair(426, "Upgrade Required"));
  statusMsg_.insert(std::make_pair(428, "Precondition Required"));
  statusMsg_.insert(std::make_pair(429, "Too Many Requests"));
  statusMsg_.insert(std::make_pair(431, "Request Header Fields Too Large"));
  statusMsg_.insert(std::make_pair(451, "Unavailable For Legal Reasons"));
  // 5xx
  statusMsg_.insert(std::make_pair(500, "Internal Server Error"));
  statusMsg_.insert(std::make_pair(501, "Not Implemented"));
  statusMsg_.insert(std::make_pair(502, "Bad Gateway"));
  statusMsg_.insert(std::make_pair(503, "Service Unavailable"));
  statusMsg_.insert(std::make_pair(504, "Gateway Timeout"));
  statusMsg_.insert(std::make_pair(505, "HTTP Version Not Supported"));
  statusMsg_.insert(std::make_pair(506, "Variant Also Negotiates"));
  statusMsg_.insert(std::make_pair(507, "Insufficient Storage"));
  statusMsg_.insert(std::make_pair(508, "Loop Detected"));
  statusMsg_.insert(std::make_pair(510, "Not Extended"));
  statusMsg_.insert(std::make_pair(511, "Network Authentication Required"));
}

void Cache::initContentType() {
  ext_contentType_map_[".html"] = "text/html";
  ext_contentType_map_[".htm"] = "text/html";
  ext_contentType_map_[".shtml"] = "text/html";
  ext_contentType_map_[".css"] = "text/css";
  ext_contentType_map_[".xml"] = "text/xml";
  ext_contentType_map_[".gif"] = "image/gif";
  ext_contentType_map_[".jpeg"] = "image/jpeg";
  ext_contentType_map_[".jpg"] = "image/jpeg";
  ext_contentType_map_[".js"] = "application/javascript";
  ext_contentType_map_[".atom"] = "application/atom+xml";
  ext_contentType_map_[".rss"] = "application/rss+xml";
  ext_contentType_map_[".mml"] = "text/mathml";
  ext_contentType_map_[".txt"] = "text/plain";
  ext_contentType_map_[".jad"] = "text/vnd.sun.j2me.app-descriptor";
  ext_contentType_map_[".wml"] = "text/vnd.wap.wml";
  ext_contentType_map_[".htc"] = "text/x-component";
  ext_contentType_map_[".avif"] = "image/avif";
  ext_contentType_map_[".png"] = "image/png";
  ext_contentType_map_[".svg"] = "image/svg+xml";
  ext_contentType_map_[".svgz"] = "image/svg+xml";
  ext_contentType_map_[".tif"] = "image/tiff";
  ext_contentType_map_[".tiff"] = "image/tiff";
  ext_contentType_map_[".wbmp"] = "image/vnd.wap.wbmp";
  ext_contentType_map_[".webp"] = "image/webp";
  ext_contentType_map_[".ico"] = "image/x-icon";
  ext_contentType_map_[".jng"] = "image/x-jng";
  ext_contentType_map_[".bmp"] = "image/x-ms-bmp";
  ext_contentType_map_[".woff"] = "font/woff";
  ext_contentType_map_[".woff2"] = "font/woff2";
  ext_contentType_map_[".jar"] = "application/java-archive";
  ext_contentType_map_[".war"] = "application/java-archive";
  ext_contentType_map_[".ear"] = "application/java-archive";
  ext_contentType_map_[".json"] = "application/json";
  ext_contentType_map_[".hqx"] = "application/mac-binhex40";
  ext_contentType_map_[".doc"] = "application/msword";
  ext_contentType_map_[".pdf"] = "application/pdf";
  ext_contentType_map_[".ps"] = "application/postscript";
  ext_contentType_map_[".eps"] = "application/postscript";
  ext_contentType_map_[".ai"] = "application/postscript";
  ext_contentType_map_[".rtf"] = "application/rtf";
  ext_contentType_map_[".m3u8"] = "application/vnd.apple.mpegurl";
  ext_contentType_map_[".kml"] = "application/vnd.google-earth.kml+xml";
  ext_contentType_map_[".kmz"] = "application/vnd.google-earth.kmz";
  ext_contentType_map_[".xls"] = "application/vnd.ms-excel";
  ext_contentType_map_[".eot"] = "application/vnd.ms-fontobject";
  ext_contentType_map_[".ppt"] = "application/vnd.ms-powerpoint";
  ext_contentType_map_[".odg"] = "application/vnd.oasis.opendocument.graphics";
  ext_contentType_map_[".odp"] = "application/vnd.oasis.opendocument.presentation";
  ext_contentType_map_[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
  ext_contentType_map_[".odt"] = "application/vnd.oasis.opendocument.text";
  ext_contentType_map_[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
  ext_contentType_map_[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
  ext_contentType_map_[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
  ext_contentType_map_[".wmlc"] = "application/vnd.wap.wmlc";
  ext_contentType_map_[".wasm"] = "application/wasm";
  ext_contentType_map_[".7z"] = "application/x-7z-compressed";
  ext_contentType_map_[".cco"] = "application/x-cocoa";
  ext_contentType_map_[".jardiff"] = "application/x-java-archive-diff";
  ext_contentType_map_[".jnlp"] = " application/x-java-jnlp-file";
  ext_contentType_map_[".run"] = "application/x-makeself";
  ext_contentType_map_[".pl"] = "application/x-perl";
  ext_contentType_map_[".pm"] = "application/x-perl";
  ext_contentType_map_[".prc"] = "application/x-pilot";
  ext_contentType_map_[".pdb"] = "application/x-pilot";
  ext_contentType_map_[".rar"] = "application/x-rar-compressed";
  ext_contentType_map_[".rpm"] = "application/x-redhat-package-manager";
  ext_contentType_map_[".sea"] = "application/x-sea";
  ext_contentType_map_[".swf"] = "application/x-shockwave-flash";
  ext_contentType_map_[".sit"] = "application/x-stuffit";
  ext_contentType_map_[".tcl"] = "application/x-tcl";
  ext_contentType_map_[".tk"] = "application/x-tcl";
  ext_contentType_map_[".der"] = "application/x-x509-ca-cert";
  ext_contentType_map_[".pem"] = "application/x-x509-ca-cert";
  ext_contentType_map_[".crt"] = "application/x-x509-ca-cert";
  ext_contentType_map_[".xpi"] = "application/x-xpinstall";
  ext_contentType_map_[".xhtml"] = "application/xhtml+xml";
  ext_contentType_map_[".xspf"] = "application/xspf+xml";
  ext_contentType_map_[".zip"] = "application/zip";
  ext_contentType_map_[".bin"] = " application/octet-stream";
  ext_contentType_map_[".exe"] = " application/octet-stream";
  ext_contentType_map_[".dll"] = " application/octet-stream";
  ext_contentType_map_[".deb"] = " application/octet-stream";
  ext_contentType_map_[".dmg"] = " application/octet-stream";
  ext_contentType_map_[".iso"] = " application/octet-stream";
  ext_contentType_map_[".img"] = " application/octet-stream";
  ext_contentType_map_[".msi"] = " application/octet-stream";
  ext_contentType_map_[".msp"] = " application/octet-stream";
  ext_contentType_map_[".msm"] = " application/octet-stream";
  ext_contentType_map_[".mid"] = "audio/midi";
  ext_contentType_map_[".midi"] = "audio/midi";
  ext_contentType_map_[".kar"] = "audio/midi";
  ext_contentType_map_[".mp3"] = "audio/mpeg";
  ext_contentType_map_[".ogg"] = "audio/ogg";
  ext_contentType_map_[".m4a"] = "audio/x-m4a";
  ext_contentType_map_[".ra"] = "audio/x-realaudio";
  ext_contentType_map_[".3gpp"] = "video/3gpp";
  ext_contentType_map_[".3gp"] = "video/3gpp";
  ext_contentType_map_[".ts"] = "video/mp2t";
  ext_contentType_map_[".mp4"] = "video/mp4";
  ext_contentType_map_[".mpeg"] = "video/mpeg";
  ext_contentType_map_[".mpg"] = "video/mpeg";
  ext_contentType_map_[".mov"] = "video/quicktime";
  ext_contentType_map_[".webm"] = "video/webm";
  ext_contentType_map_[".flv"] = "video/x-flv";
  ext_contentType_map_[".m4v"] = "video/x-m4v";
  ext_contentType_map_[".mng"] = "video/x-mng";
  ext_contentType_map_[".asx"] = "video/x-ms-asf";
  ext_contentType_map_[".asf"] = "video/x-ms-asf";
  ext_contentType_map_[".wmv"] = "video/x-ms-wmv";
  ext_contentType_map_[".avi"] = "video/x-msvideo";
}
