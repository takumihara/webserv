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

#include <iostream>
#include <map>
#include <stdexcept>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

ConnectionSocket::ConnectionSocket(int fd, Config &conf)
    : fd_(fd), conf_(conf), request_(HttpRequest(fd)), response_(HttpResponse(fd)) {}

void ConnectionSocket::handle_response(EventManager &event_manager, Config &conf) {
  std::string result = process(conf);
  request_.refresh();

  response_.createResponse(result);
  response_.sendResponse(event_manager);
}

std::string ConnectionSocket::process(Config &conf) {
  (void)conf;
  return request_.getBody();
}

void ConnectionSocket::handle_request(EventManager &event_manager) { request_.readRequest(event_manager); }
