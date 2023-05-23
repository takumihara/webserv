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

void ServerSocket::shutdown(EventManager &em) {
  close(id_);
  em.remove(std::pair<t_id, t_type>(id_, FD));
}

void ServerSocket::notify(EventManager &event_manager, struct kevent ev) {
  (void)ev;
  struct sockaddr_in add;
  int addlen;
  std::cout << "here1\n";
  int connection_fd = accept(id_, (struct sockaddr *)&add, (socklen_t *)&addlen);
  if (connection_fd == -1) {
    throw std::runtime_error("accept error");
  }
  event_manager.addChangedEvents((struct kevent){connection_fd, EVFILT_READ, EV_ADD, 0, 0, 0});
  event_manager.addChangedEvents((struct kevent){connection_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS,
                                                 EventManager::kTimeoutDuration, 0});
  ConnectionSocket *obs = new ConnectionSocket(connection_fd, port_, conf_, this);
  event_manager.add(std::pair<t_id, t_type>(connection_fd, FD), obs);
  std::cout << "here\n";
  return;
};
