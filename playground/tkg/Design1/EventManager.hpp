#ifndef EVENT_MANAGER_HPP_
#define EVENT_MANAGER_HPP_

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
#include <set>
#include <stdexcept>
#include <vector>

#include "ConnectionSocket.hpp"
#include "ServerSocket.hpp"

#define PORT 80
#define max(x, y) ((x) > (y) ? (x) : (y))

class EventManager {
 public:
  typedef std::vector<struct kevent>::iterator changed_events_iterator;
  typedef std::vector<struct kevent>::const_iterator changed_events_const_iterator;

  EventManager();
  void eventLoop();
  void addServerSocket(int fd);
  void removeServerSocket(int fd);
  void addConnectionSocket(int fd);
  void removeConnectionSocket(int fd);
  void addChangedEvents(struct kevent kevent);
  void registerServerEvent(int fd);

  static const int kTimeoutDuration = 10;

 private:
  void updateKqueue();
  void handleEvent(struct kevent ev);
  bool isServerFd(int fd);
  void clearEvlist(struct kevent *evlist);
  void handleTimeout(struct kevent ev);
  struct s_eventInfo {
    bool read;
    bool write;
    bool except;
  };

  int kq_;
  std::vector<struct kevent> changed_events_;
  std::map<int, ServerSocket *> server_sockets_;
  std::map<int, ConnectionSocket *> connection_sockets_;
  static const int kMaxEventSize = 100;
};

#endif
