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

#include "EventManager.hpp"

ConnectionSocket::ConnectionSocket(int fd) : fd_(fd), state_(kSocFree), response_size_(0), sending_response_size_(0) {}

void ConnectionSocket::handle_response(EventManager &event_manager) {
  send_response(event_manager, fd_, response_.c_str());
}

void ConnectionSocket::handle_request(EventManager &event_manager) {
  (void)state_;
  char request[100];
  int size;
  bzero(request, 100);
  if ((size = read(fd_, request, 100)) == -1) {
    std::cout << "here \n";
    throw std::runtime_error("read error");
  }
  response_size_ = size;
  if (size == 0) {
    printf("closed fd = %d\n", fd_);
    close(fd_);
    event_manager.removeConnectionSocket(fd_);
  } else {
    std::cout << "request received"
              << "(fd:" << fd_ << "): '" << request << "'" << std::endl;
    response_ = std::string(request);
    event_manager.addChangedEvents((struct kevent){fd_, EVFILT_WRITE, EV_ADD, 0, 0, 0});
    event_manager.addChangedEvents((struct kevent){fd_, EVFILT_READ, EV_DISABLE, 0, 0, 0});
    // send_response(event_manager, fd_, request);
  }
}

void ConnectionSocket::send_response(EventManager &event_manager, int socket_fd, const char *response) {
  std::cout << "sending response \n";
  int size = response_size_ - sending_response_size_;
  if (size > kWriteSize) {
    size = kWriteSize;
  } else {
    event_manager.addChangedEvents((struct kevent){socket_fd, EVFILT_WRITE, EV_DISABLE, 0, 0, 0});
    event_manager.addChangedEvents((struct kevent){socket_fd, EVFILT_READ, EV_ENABLE, 0, 0, 0});
    state_ = kSocFree;
  }
  int res = sendto(socket_fd, &response[sending_response_size_], size, 0, NULL, 0);
  sending_response_size_ += size;
  if (res == -1) {
    throw std::runtime_error("send error");
  }
  std::cout << "response sent: "
            << "'" << response << "'"
            << " (" << res << ")" << std::endl;
}
