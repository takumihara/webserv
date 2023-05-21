#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include <map>

#include "./Config/Config.hpp"

class EventManager;

class ServerSocket {
 public:
  ServerSocket(int fd, int port, Config &conf) : sock_fd_(fd), port_(port), conf_(conf) {}
  ~ServerSocket() {}
  void notify(EventManager &event_manager);
  void make_client_connection(EventManager &event_manager);

 private:
  int sock_fd_;
  int port_;
  Config conf_;
};

#endif
