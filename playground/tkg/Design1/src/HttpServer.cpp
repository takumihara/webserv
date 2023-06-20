#include "HttpServer.hpp"

#include <iostream>
#include <sstream>

#include "./Config/Cache.hpp"
#include "./Config/Config.hpp"
#include "./Config/Parser.hpp"
#include "./Config/validation.h"

sig_atomic_t sig_int = 0;

void sigIntHandler(int sig) { sig_int = sig; }

std::vector<std::string> getIPList(const std::vector<ServerConf *> servs) {
  std::vector<std::string> ip_list;
  for (std::vector<ServerConf *>::const_iterator serv = servs.cbegin(); serv != servs.cend(); serv++) {
    if ((*serv)->host_ == "0.0.0.0") {
      ip_list.clear();
      ip_list.push_back("0.0.0.0");
      return ip_list;
    }
    // not push back when current ip is already pushed back
    if (!contain(ip_list, ((*serv)->host_))) {
      ip_list.push_back((*serv)->host_);
      printf("ip: %s\n", (*serv)->host_.c_str());
    }
  }
  return ip_list;
}

uint32_t ipv4ToByte(const std::string &ip) {
  uint32_t byte = 0;
  std::vector<std::string> octets;
  std::stringstream ss(ip);
  std::string octet;

  while (std::getline(ss, octet, '.')) {
    octets.push_back(octet);
  }
  if (octets.size() != 4) throw std::runtime_error("invalid IP");
  for (std::vector<std::string>::iterator itr = octets.begin(); itr != octets.end(); itr++) {
    int oct = std::atoi(itr->c_str());
    if (0 <= oct && oct <= 255)
      byte = (byte << 8) | oct;
    else
      throw std::runtime_error("invalid IP");
  }
  return byte;
}

void HttpServer::openPorts() {
  int sock_fd;
  struct sockaddr_in add;
  for (std::map<int, std::vector<ServerConf *> >::iterator itr = conf_.port_servConf_map_.begin();
       itr != conf_.port_servConf_map_.end(); itr++) {
    std::vector<std::string> ip_list = getIPList(itr->second);
    for (std::vector<std::string>::const_iterator ip = ip_list.cbegin(); ip != ip_list.cend(); ip++) {
      if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw std::runtime_error("socket error");
      }
      setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, NULL, 0);
      fcntl(sock_fd, F_SETFL, O_NONBLOCK);
      add.sin_family = AF_INET;
      add.sin_addr.s_addr = htonl(ipv4ToByte(*ip));
      add.sin_port = htons(itr->first);

      if (bind(sock_fd, (struct sockaddr *)&add, sizeof(add)) == -1) {
        throw std::runtime_error("bind error");
      }
      if (listen(sock_fd, kBackLog) < 0) {
        throw std::runtime_error("listen error");
      }
      em_.registerServerEvent(sock_fd, itr->first, conf_);
    }
  }
  return;
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
  signal(SIGINT, sigIntHandler);
  try {
    setup();
    openPorts();
  } catch (std::runtime_error &e) {
    em_.closeAll();
    std::cerr << "Server Setup failed" << std::endl;
    exit(1);
  }
  em_.eventLoop();
}
