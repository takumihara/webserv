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

#include "AbstractSocket.hpp"

#define PORT 80
#define max(x, y) ((x) > (y) ? (x) : (y))

class EventManager {
 public:
  typedef std::set<int>::iterator set_iterator;
  typedef std::set<int>::const_iterator const_set_iterator;
  typedef std::map<int, int>::iterator map_iterator;
  typedef std::map<int, int>::const_iterator const_map_iterator;

  EventManager();
  std::set<int> &getPortFds();
  void addServerSocket(int fd);
  void removeServerSocket(int fd);
  std::set<int> &getConnectionFds();
  void addConnectionSocket(int fd);
  void removeConnectionSocket(int fd);
  std::map<int, int> &getChangedFds();
  void addChangedFd(int fd, int flag);
  void removeNewFd(int fd);
  void make_client_connection(int port_fd);
  void open_port(int kp);
  void eventLoop();
  void update_chlist(int kq);
  void update_evlist(std::vector<struct kevent> &evlist);
  struct s_eventInfo {
    bool read;
    bool write;
    bool except;
  };

 private:
  std::map<int, int> changed_fds_;
  // key: socket_fd, val: socket class
  std::map<int, AbstractSocket *> sockets_;
  static const int kEventSize = 100;
  // static int num_events_;
};

#endif
