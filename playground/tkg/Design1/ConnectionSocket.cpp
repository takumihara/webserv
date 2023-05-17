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

ConnectionSocket::ConnectionSocket(int fd) : fd_(fd), request_(HttpRequest(fd)), response_(HttpResponse(fd)) {}

void ConnectionSocket::handle_response(EventManager &event_manager) {
  std::string result = process();
  request_.refresh();

  response_.createResponse(result);
  response_.sendResponse(event_manager);
}

std::string ConnectionSocket::process() {
  return request_.getBody();
}

void ConnectionSocket::handle_request(EventManager &event_manager) { request_.readRequest(event_manager); }
