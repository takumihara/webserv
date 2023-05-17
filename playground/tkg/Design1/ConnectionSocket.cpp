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

// #include "EventManager.hpp"
#include "HttpRequest.hpp"

ConnectionSocket::ConnectionSocket(int fd)
    : fd_(fd), state_(kSocFree), request_(HttpRequest(fd)), response_size_(0), sending_response_size_(0) {}

void ConnectionSocket::handle_response(EventManager &event_manager) {
  response_ = request_.raw_data_;
  response_size_ = response_.size();
  request_.refresh();
  send_response(event_manager);
}

void ConnectionSocket::handle_request(EventManager &event_manager) { request_.readRequest(event_manager); }

void ConnectionSocket::send_response(EventManager &event_manager) {
  const char *response = response_.c_str();
  std::cout << "sending response \n";
  int size = response_size_ - sending_response_size_;
  if (size > kWriteSize) {
    size = kWriteSize;
  }
  int res = sendto(fd_, &response[sending_response_size_], size, 0, NULL, 0);
  if (res == -1) {
    perror("sendto");
    throw std::runtime_error("send error");
  }
  std::string res_str = std::string(response);
  std::cout << "response sent: "
            << "'" << res_str.substr(sending_response_size_, size) << "'"
            << " (size:" << res << ")" << std::endl;
  sending_response_size_ += size;
  if (sending_response_size_ == response_size_) {
    setToReadingState(event_manager);
  }
}

void ConnectionSocket::setToReadingState(EventManager &em) {
  sending_response_size_ = 0;
  em.addChangedEvents((struct kevent){fd_, EVFILT_WRITE, EV_DISABLE, 0, 0, 0});
  em.addChangedEvents((struct kevent){fd_, EVFILT_READ, EV_ENABLE, 0, 0, 0});
  state_ = kSocFree;
}
