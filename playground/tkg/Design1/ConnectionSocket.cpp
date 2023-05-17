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
  response_.raw_data_ = request_.raw_data_;
  response_.response_size_ = response_.raw_data_.size();
  request_.refresh();
  std::cout << "response: " << response_.raw_data_ << std::endl;
  response_.sendResponse(event_manager);
}

void ConnectionSocket::handle_request(EventManager &event_manager) { request_.readRequest(event_manager); }
