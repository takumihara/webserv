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

ConnectionSocket::ConnectionSocket(int fd) : AbstractSocket(fd), state_(kSocFree) {}

void ConnectionSocket::handle_request(EventManager &event_manager) {
  (void)state_;
  char request[100];
  int size;
  bzero(request, 100);
  if ((size = read(fd_, request, 100)) == -1) {
    throw std::runtime_error("read error");
  }
  if (size == 0) {
    printf("closed fd = %d\n", fd_);
    close(fd_);
    event_manager.removeSocket(fd_);
  } else {
    std::cout << "request received"
              << "(fd:" << fd_ << "): '" << request << "'" << std::endl;
    send_response(fd_, request);
  }
}

void ConnectionSocket::notify(EventManager &event_manager) {
  DEBUG_PUTS("ConnectionSocket notify");
  handle_request(event_manager);
}

void ConnectionSocket::send_response(int socket_fd, char *response) {
  int res = sendto(socket_fd, response, strlen(response), 0, NULL, 0);
  if (res == -1) {
    throw std::runtime_error("send error");
  }
  std::cout << "response sent: "
            << "'" << response << "'"
            << " (" << res << ")" << std::endl;
}
