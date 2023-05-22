#include "ConnectionSocket.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

ConnectionSocket::ConnectionSocket(int fd, int port, Config &conf)
    : sock_fd_(fd),
      port_(port),
      conf_(conf),
      request_(HttpRequest(fd, port, conf)),
      response_(HttpResponse(fd, port, conf)) {}

void ConnectionSocket::handle_response(EventManager &event_manager) {
  std::string result = process();
  request_.refresh();

  response_.createResponse(result);
  response_.sendResponse(event_manager);
}

static std::string readFile(const char *filename) {
  std::ifstream ifs(filename);
  if (ifs.fail()) {
    throw std::runtime_error("conf file open() failed");
  }
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

std::string ConnectionSocket::process() {
  const Config::ServerConf *serv_conf = conf_.getServConfig(port_, request_.getHeaderValue("host"));
  const Config::LocConf &loc_conf = conf_.getLocationConfig(serv_conf, request_.getRequestTarget());

  // todo: check if file exists
  const std::string path = "." + loc_conf.root_ + request_.getRequestTarget();
  DEBUG_PRINTF("path: %s\n", path.c_str());
  const std::string content = readFile(path.c_str());
  DEBUG_PUTS("PROCESSING FINISHED");
  return content;
}

void ConnectionSocket::handle_request(EventManager &event_manager) { request_.readRequest(event_manager); }
