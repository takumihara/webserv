#include "HttpServer.hpp"

#include "./Config/Cache.hpp"
#include "./Config/Config.hpp"
#include "./Config/Parser.hpp"
#include "./Config/validation.h"

int HttpServer::openPort() {
  int sock_fd;
  struct sockaddr_in add;
  for (std::map<int, std::vector<ServerConf *> >::iterator itr = conf_.port_servConf_map_.begin();
       itr != conf_.port_servConf_map_.end(); itr++) {
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      throw std::runtime_error("socket error");
    }
    add.sin_family = AF_INET;
    // todo: confのhostをuint32に変換した上でhtons()にかける
    add.sin_addr.s_addr = INADDR_ANY;
    add.sin_port = htons(itr->first);

    if (bind(sock_fd, (struct sockaddr *)&add, sizeof(add)) == -1) {
      throw std::runtime_error("bind error");
    }
    if (listen(sock_fd, 0) < 0) {
      throw std::runtime_error("listen error");
    }
    em_.registerServerEvent(sock_fd, itr->first, conf_);
  }
  return sock_fd;
}

void HttpServer::setup() {
  Parser parser;
  conf_ = parser.parse(filename_);
  conf_.makePortServConfMap();
  conf_.cache_.initCache(&conf_);
  if (!isServernameDuplicate(conf_)) {
    throw std::runtime_error("httpServer::setup: servername is duplicate");
  }
#ifdef DEBUG
  std::cout << "-----------port conf map-------------\n" << std::endl;
  conf_.printPortServConfMap();
#endif
}

void HttpServer::start() {
  setup();
  int port_fd = openPort();
  (void)port_fd;
  em_.eventLoop();
}
