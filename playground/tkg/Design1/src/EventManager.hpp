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

#include "./Config/Config.hpp"
#include "./Observee/CGI.hpp"
#include "./Observee/ConnectionSocket.hpp"
#include "./Observee/Observee.hpp"
#include "./Observee/ServerSocket.hpp"

#define PORT 80
#define max(x, y) ((x) > (y) ? (x) : (y))

class EventManager {
 public:
  typedef std::vector<struct kevent>::iterator changed_events_iterator;
  typedef std::vector<struct kevent>::const_iterator changed_events_const_iterator;

  EventManager();
  void eventLoop();
  void add(const std::pair<t_id, t_type> &key, Observee *obs);
  void remove(const std::pair<t_id, t_type> &key);
  void addChangedEvents(struct kevent kevent);
  void registerServerEvent(int fd, int port, Config &conf);
  void registerWriteEvent(uintptr_t fd);
  void registerReadEvent(uintptr_t fd);
  void disableReadEvent(uintptr_t fd);
  void disableWriteEvent(uintptr_t fd);
  void deleteTimerEvent(uintptr_t fd);
  void updateTimer(Observee *obs);
  static const int kTimeoutDuration = 5;

 private:
  void updateKqueue();
  void handleEvent(struct kevent ev);
  void clearEvlist(struct kevent *evlist);
  void handleTimeout(struct kevent ev);
  struct s_eventInfo {
    bool read;
    bool write;
    bool except;
  };

  int kq_;
  std::vector<struct kevent> changed_events_;
  std::map<std::pair<t_id, t_type>, Observee *> observees_;
  static const int kMaxEventSize = 100;
};

#endif
