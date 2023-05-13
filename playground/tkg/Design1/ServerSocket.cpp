#include "ServerSocket.hpp"

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

#include <map>
#include <set>
#include <stdexcept>

void ServerSocket::notify(EventManager &event_manager) {
  DEBUG_PUTS("ServerSocket notify\n");
  make_client_connection(event_manager);
}

void ServerSocket::make_client_connection(EventManager &event_manager) {
  struct sockaddr_in add;
  int addlen;
  int connection_fd = accept(fd_, (struct sockaddr *)&add, (socklen_t *)&addlen);
  if (connection_fd == -1) {
    throw std::runtime_error("accept error");
  }
  // event_manager.addConnectionSocket(connection_fd);
  event_manager.addChangedFd(connection_fd, EV_ADD);
  return;
}
