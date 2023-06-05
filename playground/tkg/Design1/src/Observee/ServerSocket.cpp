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

#include "../IO/FDReadCloser.hpp"

void ServerSocket::shutdown() {
  close(id_);
  em_->remove(std::pair<t_id, t_type>(id_, FD));
}

void ServerSocket::notify(struct kevent ev) {
  (void)ev;
  struct sockaddr_in add;
  int addlen;
  int connection_fd = accept(id_, (struct sockaddr *)&add, (socklen_t *)&addlen);
  if (connection_fd == -1) {
    throw std::runtime_error("accept error");
  }
  em_->addChangedEvents(EventManager::key_t(static_cast<uintptr_t>(connection_fd), EVFILT_READ),
                        (struct kevent){static_cast<uintptr_t>(connection_fd), EVFILT_READ, EV_ADD, 0, 0, 0});
  em_->addChangedEvents(EventManager::key_t(static_cast<uintptr_t>(connection_fd), EVFILT_TIMER),
                        (struct kevent){static_cast<uintptr_t>(connection_fd), EVFILT_TIMER, EV_ADD | EV_ENABLE,
                                        NOTE_SECONDS, EventManager::kTimeoutDuration, 0});
  ConnectionSocket *obs = new ConnectionSocket(connection_fd, port_, conf_, em_, this, new FDReadCloser(connection_fd));
  em_->add(std::pair<t_id, t_type>(connection_fd, FD), obs);
  return;
};
