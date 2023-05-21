#ifndef SERVER_SOCKET_HPP_
#define SERVER_SOCKET_HPP_

#include <map>

#include "./Config/Config.hpp"

class EventManager;

class ServerSocket {
 public:
  ServerSocket(int fd, Config &conf) : fd_(fd), conf_(conf) {}
  ~ServerSocket() {}
  void notify(EventManager &event_manager);
  void make_client_connection(EventManager &event_manager);

 private:
  int fd_;
  Config conf_;
};

#endif
