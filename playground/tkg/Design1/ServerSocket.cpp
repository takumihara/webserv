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

#include "EventManager.hpp"

// todo: remove
void ServerSocket::notify(EventManager &event_manager) {
  DEBUG_PUTS("ServerSocket notify\n");
  make_client_connection(event_manager);
}

void ServerSocket::make_client_connection(EventManager &event_manager) {
  struct sockaddr_in add;
  int addlen;
  int connection_fd = accept(sock_fd_, (struct sockaddr *)&add, (socklen_t *)&addlen);
  if (connection_fd == -1) {
    throw std::runtime_error("accept error");
  }
  event_manager.addChangedEvents((struct kevent){connection_fd, EVFILT_READ, EV_ADD, 0, 0, 0});
  event_manager.addChangedEvents((struct kevent){connection_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS,
                                                 EventManager::kTimeoutDuration, 0});
  event_manager.addConnectionSocket(connection_fd, port_, conf_);
  return;
}
