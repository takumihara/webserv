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

#include "ConnectionSocket.hpp"
#include "ServerSocket.hpp"

#define PORT 80
#define max(x, y) ((x) > (y) ? (x) : (y))

typedef enum SockType {
  kTypeServer,
  kTypeConnection,
} t_socktype;

struct SockInfo {
  t_socktype type;
  int flags;
};

class EventManager {
 public:
  typedef std::set<int>::iterator set_iterator;
  typedef std::set<int>::const_iterator const_set_iterator;
  typedef std::map<int, SockInfo>::iterator map_iterator;
  typedef std::map<int, SockInfo>::const_iterator const_map_iterator;

  EventManager();
  void eventLoop();
  void addServerSocket(int fd);
  void removeServerSocket(int fd);
  void addConnectionSocket(int fd);
  void removeConnectionSocket(int fd);
  void addChangedFd(int fd, SockInfo info);
  void registerServerEvent(int fd);

 private:
  void updateKqueue();
  void handleEvent(int fd);
  bool isServerFd(int fd);
  void clearEvlist(struct kevent *evlist);
  struct s_eventInfo {
    bool read;
    bool write;
    bool except;
  };

  int kq_;
  std::map<int, SockInfo> changed_fds_;
  std::map<int, ServerSocket *> server_sockets_;
  std::map<int, ConnectionSocket *> connection_sockets_;
  static const int kMaxEventSize = 100;
};

#endif
